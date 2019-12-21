#include "Chunk.hh"

#include <set>
#include <cassert>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include "Vertices.hh"
#include "World.hh"

using namespace glow;


Chunk::Chunk(tg::ipos3 chunkPos, int size, World *world) : chunkPos(chunkPos), size(size), world(world)
{
    mBlocks.resize(size * size * size);
}

SharedChunk Chunk::create(tg::ipos3 chunkPos, int size, World *world)
{
    if (chunkPos.x % size != 0 || chunkPos.y % size != 0 || chunkPos.z % size != 0)
        glow::error() << "Position must be a multiple of size!";

    // "new" because Chunk() is private
    return std::shared_ptr<Chunk>(new Chunk(chunkPos, size, world));
}

///
/// Create the meshes for this chunk. (one per material)
/// The return value is a map from material to mesh.
///
/// Your job is to:
///     - enhance the performance by (re-)creating meshes only if needed
///       (Dirty-flagging can be done using mIsDirty)
///     - create faces for all visible blocks
///     - adapt Vertices.hh (vertex type) and terrain.vsh (vertex shader)
///
///     - Advanced: do not create faces that are never visible from the outside
///                 - no face between two solids
///                 - no face between two translucent materials of the same type
///     - Advanced: compute some fake ambient occlusion for every vertex
///                 (also optimize the triangulation of the quads depending on the ao values)
///     - Advanced: Pack vertex data in 16 byte or less (e.g. a single (tg::)pos4)
///
///
/// Notes:
///     - pre-filled code is meant as a starting helper; you may change everything
///       you want inside the strips, e.g. implement helper functions (see Chunk.hh)
///     - for every material (except air) in a chunk, one mesh must be created
///     - it is up to you whether you create an indexed mesh or not
///     - local coordinates: 0..size-1            -> block query: block(localPos)
///     - global coordinates: chunkPos + localPos -> block query: queryBlock(globalPos)
///     - read Chunk.hh for helpers and explanations
///     - for AO see screenshots, "Fake Ambient Occlusion.jpg", and "Fake AO - Triangulation.jpg"
///     - don't forget that some faces need information from neighboring chunks (global queries)
///
/// ============= STUDENT CODE BEGIN =============
std::map<int, SharedVertexArray> Chunk::queryMeshes()
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    if (!this->isDirty()) return mMeshes;

    // clear list of cached meshes
    mMeshes.clear();
    std::set<int> built; // track already built materials

    // ensure that each material is accounted for
    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                tg::ivec3 localPos = { x, y, z };
                auto const &b = block(localPos); // get block

                // if block material is not air and not already built
                if (!b.isAir() && !built.count(b.mat))
                {
                    // mark as built
                    built.insert(b.mat);

                    // create VAO
                    auto vao = buildMeshFor(b.mat);
                    if (vao) // might be nullptr if fully surrounded
                        mMeshes[b.mat] = vao;
                }
            }

    this->setDirty(false);

    glow::info() << "Rebuilt mesh for " << chunkPos;
    return mMeshes;
}

int Chunk::getVtxIdx(tg::ivec3 offset) const
{
	return offset.x * 4 + offset.y * 2 + offset.z;
}

SharedVertexArray Chunk::buildMeshFor(int mat) const
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> vertices;

    for (auto z = 0; z < size; ++z) {

        for (auto y = 0; y < size; ++y) {

            for (auto x = 0; x < size; ++x) {

                tg::ivec3 localPos = { x, y, z };  // local position

                auto globalPos = chunkPos + localPos;  // global position

                if (block(localPos).mat != mat) continue;  // consider only current material

                for (auto direction : { -1, 1 }) {

                    for (auto axis : { 0, 1, 2 }) {

                        auto normal = direction * tg::ivec3(axis == 0, axis == 1, axis == 2);

                        bool that_solid = this->queryBlock(globalPos + normal).isSolid();

                        bool same_mat = block(localPos).mat == this->queryBlock(globalPos + normal).mat;

                        if (that_solid | same_mat) continue;

                        auto vtxA = tg::ivec3((axis == 0) * (direction == 1), (axis == 0) * 0, (axis == 0) * 0);
                        auto vtxB = tg::ivec3((axis == 0) * (direction == 1), (axis == 0) * 0, (axis == 0) * 1);
                        auto vtxD = tg::ivec3((axis == 0) * (direction == 1), (axis == 0) * 1, (axis == 0) * 0);
                        auto vtxC = tg::ivec3((axis == 0) * (direction == 1), (axis == 0) * 1, (axis == 0) * 1);

                        vtxA += tg::ivec3((axis == 1) * 0, (axis == 1) * (direction == 1), (axis == 1) * 0);
                        vtxB += tg::ivec3((axis == 1) * 1, (axis == 1) * (direction == 1), (axis == 1) * 0);
                        vtxD += tg::ivec3((axis == 1) * 0, (axis == 1) * (direction == 1), (axis == 1) * 1);
                        vtxC += tg::ivec3((axis == 1) * 1, (axis == 1) * (direction == 1), (axis == 1) * 1);

                        vtxA += tg::ivec3((axis == 2) * 0, (axis == 2) * 0, (axis == 2) * (direction == 1));
                        vtxB += tg::ivec3((axis == 2) * 0, (axis == 2) * 1, (axis == 2) * (direction == 1));
                        vtxD += tg::ivec3((axis == 2) * 1, (axis == 2) * 0, (axis == 2) * (direction == 1));
                        vtxC += tg::ivec3((axis == 2) * 1, (axis == 2) * 1, (axis == 2) * (direction == 1));

                        float aoA = aoAt(globalPos, vtxA, normal);
                        float aoB = aoAt(globalPos, vtxB, normal);
                        float aoC = aoAt(globalPos, vtxC, normal);
                        float aoD = aoAt(globalPos, vtxD, normal);

                        int osA = getVtxIdx(vtxA);
                        int osB = getVtxIdx(vtxB);
                        int osC = getVtxIdx(vtxC);
                        int osD = getVtxIdx(vtxD);

                        if (direction == -1) {

                            if (fabs(aoB - aoD) > fabs(aoA - aoC)) {
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                            } else {
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                            }
                        }
                        else {

                            if (fabs(aoA - aoC) < fabs(aoB - aoD)) {
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                            } else {
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                                vertices.push_back(TerrainVertex(globalPos, axis, aoA, osA, osB, osC, osD, 0));  // A
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoD, osA, osB, osC, osD, 3));  // D
                                vertices.push_back(TerrainVertex(globalPos, axis, aoC, osA, osB, osC, osD, 2));  // C
                                vertices.push_back(TerrainVertex(globalPos, axis, aoB, osA, osB, osC, osD, 1));  // B
                            }
                        }
                    }
                }
            }
        }
    }

    if (vertices.empty()) return nullptr;

    auto ab = ArrayBuffer::create(TerrainVertex::attributes());
    ab->bind().setData(vertices);

    glow::info() << "Created " << vertices.size() << " verts for mat " << mat << " in chunk " << chunkPos;
    return VertexArray::create(ab, GL_TRIANGLES);
}

float Chunk::aoAt(tg::ipos3 pos, tg::ivec3 vtx, tg::ivec3 normal) const
{
    auto diagonal = vtx - (1 - vtx) - normal;

    auto bidiagonal = cross(diagonal, normal);

    auto dx = tg::ivec3(normalize(diagonal + bidiagonal));
    auto dy = tg::ivec3(normalize(diagonal - bidiagonal));

    return aoAt(pos, dx, dy, normal);
}

float Chunk::aoAt(tg::ipos3 pos, tg::ivec3 dx, tg::ivec3 dy, tg::ivec3 dz) const
{
    auto p00 = queryBlock(pos + dz);
    auto p01 = queryBlock(pos + dy + dz);
    auto p11 = queryBlock(pos + dx + dy + dz);
    auto p10 = queryBlock(pos + dx + dz);

    assert (!p00.isSolid());

    if (p10.isSolid() & p01.isSolid()) return 0.0;

    return (3.0 - p10.isSolid() - p01.isSolid() - p11.isSolid()) / 3.0;
}
/// ============= STUDENT CODE END =============

void Chunk::markDirty()
{
    mIsDirty = true;

    mMeshes.clear();
}

const Block &Chunk::queryBlock(tg::ipos3 worldPos) const
{
    if (contains(worldPos))
        return block(worldPos - chunkPos);
    else
        return world->queryBlock(worldPos);
}
