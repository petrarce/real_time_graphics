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

    if(!isDirty()){
        return this->mMeshes;
    }
    // clear list of cached meshes
    mMeshes.clear();
    std::set<int> built; // track already built materials

    // ensure that each material is accounted for
    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                tg::ivec3 rp = { x, y, z };
                auto const &b = block(rp); // get block

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

    this->mIsDirty = false;
    glow::info() << "Rebuilding mesh for " << chunkPos;
    return mMeshes;
}

SharedVertexArray Chunk::buildMeshFor(int mat) const
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> vertices;

    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                tg::ivec3 rp = { x, y, z }; // local position
                tg::ipos3 gp = chunkPos + rp;     // global position
                auto const &blk = block(rp);

                if (blk.mat != mat)
                    continue; // consider only current material

                // go over all 6 directions
                for (auto s : { -1, 1 }){
                    tg::ipos3 ptPos = gp - tg::ivec3(1,1,1)*((-1-s)/2);
                    for (auto dir : { 0, 1, 2 })
                    {
                        // face normal
                        //glow::info() << "setting new pair of triangles";
                        auto n = s * tg::ivec3(dir == 0, dir == 1, dir == 2);
                        auto neighborBlock = rp + n;
                        if(neighborBlock[0] < size && neighborBlock[0] > 0 &&
                            neighborBlock[1] < size && neighborBlock[1] > 0 &&
                            neighborBlock[2] < size && neighborBlock[2] > 0 &&
                            block(neighborBlock).mat > 0 ){
                            continue;
                        }
                        auto tg1 = -s * tg::ivec3(dir == 2, dir == 0, dir == 1);
                        auto tg2 = -s * tg::ivec3(dir == 1, dir == 2, dir == 0);
                        //glow::info() << "gp=" << gp;
                        //glow::info() << "ptPos=" << ptPos;
                        //glow::info() << "n=" << n << ", tg1=" << tg1 << ", th2=" << tg2;
                        // TODO!
                        tg::ipos3 vert1 = ptPos;
                        tg::ipos3 vert2 = ptPos + tg1;
                        tg::ipos3 vert3 = ptPos + tg2;

                        tg::ipos3 vert4 = ptPos + tg1;
                        tg::ipos3 vert5 = ptPos + tg2;
                        tg::ipos3 vert6 = ptPos + tg1 + tg2;
                        //glow::info() << "v1=" << vert1 << ", v2=" << vert2 << ", v3=" << vert3;
                        //glow::info() << "v4=" << vert4 << ", v5=" << vert5 << ", v6=" << vert6;
                        TerrainVertex tmp;
                        tmp.pos = tg::pos3(vert1[0], vert1[1], vert1[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 0<<16;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert2[0], vert2[1], vert2[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 1<<16;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert3[0], vert3[1], vert3[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 2<<16;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert4[0], vert4[1], vert4[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 1<<16;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert5[0], vert5[1], vert5[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 2<<16;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert6[0], vert6[1], vert6[2]);
                        tmp.texAndNormalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1))<<24;
                        tmp.texAndNormalType |= 3<<16;
                        vertices.push_back(tmp);

                    }
                }
            }

    if (vertices.empty())
        return nullptr; // no visible faces

    auto ab = ArrayBuffer::create(TerrainVertex::attributes());
    ab->bind().setData(vertices);

    glow::info() << "Created " << vertices.size() << " verts for mat " << mat << " in chunk " << chunkPos;
    return VertexArray::create(ab);
}

float Chunk::aoAt(tg::ipos3 pos, tg::ivec3 dx, tg::ivec3 dy) const
{
    return 1.0f; // TODO
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
