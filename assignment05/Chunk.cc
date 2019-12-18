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

int Chunk::getAOtype(const tg::ivec3& pt, const tg::ivec3& n, const tg::ivec3& tg1, const tg::ivec3& tg2) const
{
    auto body1pt = pt + n + tg1;
    auto body2pt = pt + n + tg2 + tg1;
    auto body3pt = pt + n + tg2;
    bool body1 = ((body1pt.x < size && body1pt.x >= 0) && 
                    (body1pt.y < size && body1pt.y >= 0) && 
                    (body1pt.z < size && body1pt.z >= 0) && 
                    (block(body1pt).mat != 0));
    bool body2 = ((body2pt.x < size && body2pt.x >= 0) && 
                    (body2pt.y < size && body2pt.y >= 0) && 
                    (body2pt.z < size && body2pt.z >= 0) && 
                    (block(body2pt).mat != 0));
    bool body3 = ((body3pt.x < size && body3pt.x >= 0) && 
                    (body3pt.y < size && body3pt.y >= 0) && 
                    (body3pt.z < size && body3pt.z >= 0) && 
                    (block(body3pt).mat != 0));
    if(body1 && body3){
        return 0;
    } else if((body1 && body2) || (body2 && body3)){
        return 1;
    } else if(body1 || body3 || body2){
        return 2;
    }
    return 3;
}

SharedVertexArray Chunk::buildMeshFor(int mat) const
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> vertices;
    int dbgcnt = 0;
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
                tg::ivec3 normals[3] = {tg::ivec3(1,0,0),tg::ivec3(0,1,0), tg::ivec3(0,0,1)};
                tg::ivec3 tangens1[3] = {tg::ivec3(0,-1,0), tg::ivec3(0,0,-1), tg::ivec3(-1,0,0)};
                tg::ivec3 tangens2[3] = {tg::ivec3(0,0,-1), tg::ivec3(-1,0,0), tg::ivec3(0,-1,0)};
                //glow::info() << "build new box";
                for (int s : { -1, 1 }){
                    for (int dir : { 0, 1, 2 })
                    {
                        // face normal
                        tg::ivec3 n = s * normals[dir];
                        tg::ipos3 ptPos = gp + tg::ivec3(1,1,1)*(1+s)/2;

                        auto neighborBlock = rp + n;
                        if(neighborBlock[0] < size && neighborBlock[0] > 0 &&
                            neighborBlock[1] < size && neighborBlock[1] > 0 &&
                            neighborBlock[2] < size && neighborBlock[2] > 0 &&
                            block(neighborBlock).mat > 0 ){
                            continue;
                        }
                        //glow::info() << "build new fase";

                        auto tg1 = s * tangens1[dir];
                        auto tg2 = s * tangens2[dir];
                        // TODO!
                        tg::ipos3 vert1 = ptPos;
                        tg::ipos3 vert2;
                        tg::ipos3 vert3;
                        if(s > 0){
                            vert2 = ptPos + tg1;
                            vert3 = ptPos + tg2;
                        } else {
                            vert2 = ptPos + tg2;
                            vert3 = ptPos + tg1;
                        }
                        tg::ipos3 vert4 = ptPos + tg1 + tg2;
                        if(dbgcnt < 20){
                            glow::info() << "gp=" << gp;
                            glow::info() << "n=" << n;
                            glow::info() << "tg1=" << tg1;
                            glow::info() << "tg2=" << tg2;
                            glow::info() << "vert1=" << vert1;
                            glow::info() << "vert2=" << vert2;
                            glow::info() << "vert3=" << vert3;
                            glow::info() << "vert4=" << vert4;
                            dbgcnt++;
                        }
                        TerrainVertex tmp;
                        int normalType = (0*(n[2]==1) + 
                                                1*(n[1]==1) +
                                                2*(n[0]==1) +
                                                3*(n[2]==-1) + 
                                                4*(n[1]==-1) +
                                                5*(n[0]==-1));

                        tmp.pos = tg::pos3(vert1[0], vert1[1], vert1[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (0<<16&0x00ff0000);
                        tmp.texAndNormalType |= getAOtype(rp, n, -tg1, -tg2)<<8;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert2[0], vert2[1], vert2[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (2<<16&0x00ff0000);
                        if(s > 0){
                            tmp.texAndNormalType |= getAOtype(rp, n, tg1, -tg2)<<8;
                        } else {
                            tmp.texAndNormalType |= getAOtype(rp, n, -tg1, tg2)<<8;
                        }
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert4[0], vert4[1], vert4[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (3<<16&0x00ff0000);
                        tmp.texAndNormalType |= getAOtype(rp, n, tg1, tg2)<<8;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert1[0], vert1[1], vert1[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (0<<16&0x00ff0000);
                        tmp.texAndNormalType |= getAOtype(rp, n, -tg1, -tg2)<<8;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert4[0], vert4[1], vert4[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (3<<16&0x00ff0000);
                        tmp.texAndNormalType |= getAOtype(rp, n, tg1, tg2)<<8;
                        vertices.push_back(tmp);

                        tmp.pos = tg::pos3(vert3[0], vert3[1], vert3[2]);
                        tmp.texAndNormalType = normalType<<24;
                        tmp.texAndNormalType |= (1<<16&0x00ff0000);
                        if(s > 0){
                            tmp.texAndNormalType |= getAOtype(rp, n, -tg1, tg2)<<8;
                        } else {
                            tmp.texAndNormalType |= getAOtype(rp, n, tg1, -tg2)<<8;
                           
                        }
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
