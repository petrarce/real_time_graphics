#include "TerrainWorker.hh"

#include <chrono>

#include <typed-geometry/tg-lean.hh>

#include <glow/common/log.hh>

#include "Chunk.hh"
#include "MeshGenerator.hh"
#include "World.hh"

TerrainWorker::TerrainWorker(World* world) : mWorld(world)
{
    // launch thread here (after member init)
    mWorkerThread = std::thread(
        [](TerrainWorker* w) {
            w->run(); // execute thread
        },
        this);
}

void TerrainWorker::stop()
{
    mShouldStop = true;
    mWorkerThread.join();
}

void TerrainWorker::update()
{
    if (mJobsGenFinished.empty() && mJobsMeshFinished.empty())
        return; // early out

    mMutexFinished.lock();

    // process generation jobs
    for (auto c : mJobsGenFinished)
        mWorld->notifyChunkGenerated(c.chunk);
    mJobsGenFinished.clear();

    // process mesh jobs
    for (auto c : mJobsMeshFinished)
        mWorld->notifyChunkMeshed(c.chunk, c.data);
    mJobsMeshFinished.clear();

    mMutexFinished.unlock();
}

void TerrainWorker::enqueueGen(SharedChunk chunk)
{
    mMutexNew.lock();
    mJobsGen.push({chunk});
    mMutexNew.unlock();
}

void TerrainWorker::enqueueMesh(SharedChunk chunk, std::vector<Block> blocks)
{
    mMutexNew.lock();
    mJobsMesh.push({chunk, std::move(blocks), chunk->getMeshVersion()});
    mMutexNew.unlock();
}

void TerrainWorker::run()
{
    while (!mShouldStop)
    {
        auto doneWork = false;

        // generate
        if (!mJobsGen.empty())
        {
            // get a job
            mMutexNew.lock();
            auto job = std::move(mJobsGen.front());
            mJobsGen.pop();
            mMutexNew.unlock();

            // process job
            mWorld->generate(*job.chunk);
            doneWork = true;

            // finish job
            mMutexFinished.lock();
            mJobsGenFinished.push_back({job.chunk});
            mMutexFinished.unlock();
        }

        // mesh
        if (!mJobsMesh.empty())
        {
            mMutexNew.lock();
            auto job = std::move(mJobsMesh.front());
            mJobsMesh.pop();
            mMutexNew.unlock();

            // only if current mesh version
            if (job.chunk->getMeshVersion() == job.version)
            {
                // process job
                auto meshes = generateMesh(job.blocks, job.chunk->chunkPos);
                doneWork = true;

                // finish job
                mMutexFinished.lock();
                mJobsMeshFinished.push_back({ job.chunk, std::move(meshes) });
                mMutexFinished.unlock();
            }
        }

        // sleep if no work done
        if (!doneWork)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
