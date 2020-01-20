#include "Chunk.hh"

#include <set>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include "Vertices.hh"
#include "World.hh"

using namespace glow;


Chunk::Chunk(tg::ipos3 chunkPos, World *world) : chunkPos(chunkPos), world(world)
{
    mBlocks.resize(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, Block::invalid());
}

SharedChunk Chunk::create(tg::ipos3 chunkPos, World *world)
{
    if (chunkPos.x % CHUNK_SIZE != 0 || chunkPos.y % CHUNK_SIZE != 0 || chunkPos.z % CHUNK_SIZE != 0)
        glow::error() << "Position must be a multiple of size!";

    // "new" because Chunk() is private
    return std::shared_ptr<Chunk>(new Chunk(chunkPos, world));
}

const std::vector<TerrainMesh> &Chunk::queryMeshes()
{
    // mesh update is done in terrain worker
    return mMeshes;
}

void Chunk::markDirty()
{
    if (!mIsDirty)
        world->notifyDirtyChunk(this); // enqueue CPU update

    mIsDirty = true;

    // Don't clear meshes: they might get re-used
    // mMeshes.clear();
}

void Chunk::update()
{
    if (!mIsDirty)
        return; // nothing to do

    mActiveLightFountains.clear();

    // recalc flags
    mIsFullyAir = true;
    mIsFullySolid = true;

    // recalc aabb
    mAabbMin = tg::pos3((float)CHUNK_SIZE + 1);
    mAabbMax = tg::pos3(-1.0f);

    tg::ivec3 amin(CHUNK_SIZE + 1);
    tg::ivec3 amax(-1);

    auto idx = 0;
    auto pChunk = mBlocks.data();
    auto fullAir = true;
    auto fullSolid = true;
    for (auto z = 0; z < CHUNK_SIZE; ++z)
        for (auto y = 0; y < CHUNK_SIZE; ++y)
            for (auto x = 0; x < CHUNK_SIZE; ++x)
            {
                auto const &b = pChunk[idx++];
                tg::ivec3 p = {x, y, z}; // relative local position

                // update flags
                if (!b.isAir())
                    fullAir = false;
                if (!b.isSolid())
                    fullSolid = false;

                // update aabb
                amax = tg::max(p, amax);
                amin = tg::min(p, amin);

                if (b.isInvalid() || b.isAir())
                    continue;

                // gather light fountains
                if (world->getMaterialFromIndex(b.mat)->spawnsLightSources)
                {
                    auto gp = chunkPos + p;
                    if (queryBlock(gp + tg::ivec3(0, 1, 0)).isAir())
                    {
                        // air above this block
                        mActiveLightFountains.push_back(gp);
                    }
                }
            }

    mAabbMin = tg::pos3(chunkPos + amin);
    mAabbMax = tg::pos3(chunkPos + amax + 1);

    mIsFullyAir = fullAir;
    mIsFullySolid = fullSolid;

    mIsDirty = false;
}

void Chunk::notifyMeshData(const std::vector<TerrainMeshData> &meshData)
{
    GLOW_ACTION();

    // upload new meshes
    std::vector<TerrainMesh> newMeshes;

    for (auto const &data : meshData)
    {
        if (data.vertexData.empty())
            continue; // no data

        auto d = data.dir;
        auto pd = tg::abs(d);
        auto s = (d.x + d.y + d.z + 1) / 2;
        auto pdir = pd.y + pd.z * 2 + s * 3;
        TG_ASSERT(0 <= pdir && pdir < 6);

        TerrainMesh mesh;
        mesh.mat = world->getMaterialFromIndex(data.mat)->renderMaterials[pdir];
        mesh.dir = data.dir;
        mesh.aabbMin = data.aabbMin;
        mesh.aabbMax = data.aabbMax;

        // try to re-use existing mesh
        for (auto const &m : mMeshes)
            if (m.mat == mesh.mat && m.dir == mesh.dir)
            {
                mesh.abData = m.abData;
                mesh.abPositions = m.abPositions;
                mesh.vaoFull = m.vaoFull;
                mesh.vaoPosOnly = m.vaoPosOnly;
                break;
            }

        // .. otherwise create VAO/AB
        if (!mesh.vaoFull)
        {
            mesh.abPositions = ArrayBuffer::create();
            mesh.abPositions->defineAttribute<tg::pos3>("aPosition");
            mesh.abData = ArrayBuffer::create(TerrainVertex::attributes());
            mesh.vaoFull = VertexArray::create({mesh.abPositions, mesh.abData});
            mesh.vaoPosOnly = VertexArray::create(mesh.abPositions);
        }

        // upload new vertex data
        mesh.abPositions->bind().setData(data.vertexPositions);
        mesh.abData->bind().setData(data.vertexData);

        // add to result
        newMeshes.push_back(mesh);
    }

    // replace old meshes
    mMeshes = newMeshes;
    // glow::info() << "new meshes for " << chunkPos;
}

const Block &Chunk::queryBlock(tg::ipos3 worldPos) const
{
    if (contains(worldPos))
        return block(worldPos - chunkPos);
    else
        return world->queryBlock(worldPos);
}

std::vector<Material const *> Chunk::queryMaterials() const
{
    std::set<int8_t> matIdx;

    for (auto const &b : mBlocks)
        matIdx.insert(b.mat);

    std::vector<Material const *> mats;

    for (auto m : matIdx)
    {
        if (m == 0)
            mats.push_back(nullptr);
        else
            mats.push_back(world->getMaterialFromIndex(m));
    }

    return mats;
}
