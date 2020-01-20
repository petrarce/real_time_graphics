#pragma once

#include <queue>
#include <mutex>
#include <thread>

#include <glow/common/shared.hh>

#include "Block.hh"
#include "TerrainMesh.hh"

class World;
GLOW_SHARED(class, Chunk);

/**
 * @brief Separate thread for generating and creating chunks
 */
class TerrainWorker
{
private:
    struct GenJob
    {
        SharedChunk chunk;
    };
    struct GenJobFin
    {
        SharedChunk chunk;
    };

    struct MeshJob
    {
        SharedChunk chunk;
        std::vector<Block> blocks;
        int version;
    };
    struct MeshJobFin
    {
        SharedChunk chunk;
        std::vector<TerrainMeshData> data;
    };

private:
    /// true iff the worker should stop
    volatile bool mShouldStop = false;

    /// separate worker thread
    std::thread mWorkerThread;

    /// backref to the world
    World* mWorld;

    // queues
    std::mutex mMutexNew;
    std::mutex mMutexFinished;

    std::queue<GenJob> mJobsGen;
    std::vector<GenJobFin> mJobsGenFinished;

    std::queue<MeshJob> mJobsMesh;
    std::vector<MeshJobFin> mJobsMeshFinished;

public:
    TerrainWorker(World* world);

    /// stops this worker
    void stop();

    /// processes all finished jobs
    void update();

    // enqueue a new job
    void enqueueGen(SharedChunk chunk);
    void enqueueMesh(SharedChunk chunk, std::vector<Block> blocks);

private:
    /// thread executio
    void run();
};
