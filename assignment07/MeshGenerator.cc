#include "MeshGenerator.hh"

#include <cassert>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>
#include <typed-geometry/tg.hh>

#include "Constants.hh"

#define EXT_SIZE (CHUNK_SIZE + 2)

namespace
{
int aoAt(const std::vector<Block> &blocks, tg::ipos3 pos, tg::ivec3 dx, tg::ivec3 dy)
{
    auto block = [&blocks](tg::ipos3 ip) { return blocks[(ip.z * EXT_SIZE + ip.y) * EXT_SIZE + ip.x]; };

    // block(pos) is non-solid

    // query three relevant materials
    auto s10 = block(pos + dx).isSolid();
    auto s01 = block(pos + dy).isSolid();
    auto s11 = block(pos + dx + dy).isSolid();

    if (s10 && s01)
        s11 = true; // corner case

    return 3 - s10 - s01 - s11;
}

int edgeAt(const std::vector<Block> &blocks, tg::ipos3 pos, tg::ivec3 d, tg::ivec3 n)
{
    auto block = [&blocks](tg::ipos3 ip) { return blocks[(ip.z * EXT_SIZE + ip.y) * EXT_SIZE + ip.x]; };

    auto sTop = block(pos + d).isSolid();
    auto sBot = block(pos + d - n).isSolid();

    if (sTop)
        return 2;
    else if (sBot)
        return 1;
    else
        return 0;
}

void buildMeshFor(const std::vector<Block> &blocks,
                  tg::ipos3 chunkPos,
                  int mat, //
                  std::vector<TerrainMeshData> &newMeshes)
{
    auto block = [&blocks](tg::ipos3 ip) { return blocks[(ip.z * EXT_SIZE + ip.y) * EXT_SIZE + ip.x]; };

    // GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> dataPerMesh[6];
    std::vector<tg::pos3> positionsPerMesh[6];
    TerrainMeshData meshes[6];

    // set mesh dir
    for (auto pdir = 0; pdir < 6; ++pdir)
    {
        auto dir = pdir % 3;
        auto s = pdir < 3 ? -1 : 1;

        // face normal/direction
        meshes[pdir].dir = s * tg::ivec3(dir == 0, dir == 1, dir == 2);
    }

    // set mesh AB and VAO and AABB
    for (auto mi = 0; mi < 6; ++mi)
    {
        auto &mesh = meshes[mi];

        // set mat
        mesh.mat = mat;

        // prepare AABB
        mesh.aabbMin = tg::pos3(chunkPos) + CHUNK_SIZE + 1;
        mesh.aabbMax = tg::pos3(chunkPos);
    }

    // optimized packed vertex
    auto addVert = [&](tg::pos3 pos, int pdir, int vIdx, tg::ivec4 ao, tg::ivec4 edges) {
        positionsPerMesh[pdir].push_back(pos);

        // CAUTION: flag assembly in OPPOSITE direction
        int flags = 0;

        // edges
        flags = flags * 3 + edges.w;
        flags = flags * 3 + edges.z;
        flags = flags * 3 + edges.y;
        flags = flags * 3 + edges.x;

        // AO
        flags = flags * 4 + ao.w;
        flags = flags * 4 + ao.z;
        flags = flags * 4 + ao.y;
        flags = flags * 4 + ao.x;

        // packed direction
        flags = flags * 6 + pdir;

        // vertex idx
        flags = flags * 4 + vIdx;

        // assemble "vertex"
        TerrainVertex v;
        v.flags = flags;
        dataPerMesh[pdir].push_back(v);
    };

    for (auto z = 1; z <= CHUNK_SIZE; ++z)
        for (auto y = 1; y <= CHUNK_SIZE; ++y)
            for (auto x = 1; x <= CHUNK_SIZE; ++x)
            {
                tg::ipos3 p = {x, y, z}; // local position
                auto const &blk = block(p);

                if (blk.mat != mat)
                    continue; // consider only current material

                auto gp = chunkPos + tg::ivec3(p) - 1; // global position

                // go over all directions
                // packed dir 0,1,2 negative, 3,4,5 positive
                for (auto pdir = 0; pdir < 6; ++pdir)
                {
                    auto dir = pdir % 3;
                    auto n = meshes[pdir].dir;

                    // neighbor position and block
                    auto np = p + n;
                    auto nblk = block(np); // "global" query

                    // check if we have to gen a face
                    bool hasFace = !nblk.isSolid() && nblk.mat != mat;

                    if (hasFace)
                    {
                        auto dn = tg::vec3(n);
                        auto dt = cross(dn, tg::vec3(dir == 1, dir == 2, dir == 0));
                        auto db = cross(dt, dn);

                        auto idt = tg::ivec3(dt);
                        auto idb = tg::ivec3(db);

                        auto pc = tg::pos3(gp) + 0.5f + dn * 0.5f;

                        // vertex indixes
                        auto i00 = 0;
                        auto i01 = 1;
                        auto i10 = 2;
                        auto i11 = 3;

                        // Calculate position
                        auto p00 = pc - dt * 0.5f - db * 0.5f;
                        auto p01 = pc - dt * 0.5f + db * 0.5f;
                        auto p10 = pc + dt * 0.5f - db * 0.5f;
                        auto p11 = pc + dt * 0.5f + db * 0.5f;

                        // Ambient Occlusion trick
                        auto a00 = aoAt(blocks, p + n, -idt, -idb);
                        auto a01 = aoAt(blocks, p + n, -idt, +idb);
                        auto a10 = aoAt(blocks, p + n, +idt, -idb);
                        auto a11 = aoAt(blocks, p + n, +idt, +idb);

                        tg::ivec4 ao = {a00, a01, a10, a11};

                        // Edge tricks
                        auto ePT = edgeAt(blocks, p + n, +idt, n);
                        auto eNT = edgeAt(blocks, p + n, -idt, n);
                        auto ePB = edgeAt(blocks, p + n, +idb, n);
                        auto eNB = edgeAt(blocks, p + n, -idb, n);

                        auto edges = tg::ivec4(ePT, eNT, ePB, eNB);
                        // auto e00 = glm::ivec2(eNT, eNB);
                        // auto e01 = glm::ivec2(eNT, ePB);
                        // auto e10 = glm::ivec2(ePT, eNB);
                        // auto e11 = glm::ivec2(ePT, ePB);

                        // Create face
                        addVert(p00, pdir, i00, ao, edges);
                        addVert(p01, pdir, i01, ao, edges);
                        addVert(p11, pdir, i11, ao, edges);

                        addVert(p00, pdir, i00, ao, edges);
                        addVert(p11, pdir, i11, ao, edges);
                        addVert(p10, pdir, i10, ao, edges);
                    }
                }
            }

    // create GPU data and write results
    for (auto pdir = 0; pdir < 6; ++pdir)
    {
        auto &mesh = meshes[pdir];

        if (dataPerMesh[pdir].empty()) // no visible faces
            continue;

        // move new vertex data
        mesh.vertexData = std::move(dataPerMesh[pdir]);
        mesh.vertexPositions = std::move(positionsPerMesh[pdir]);

        // update AABB
        for (auto const &pos : mesh.vertexPositions)
        {
            mesh.aabbMin = tg::min(pos, mesh.aabbMin);
            mesh.aabbMax = tg::max(pos, mesh.aabbMax);
        }

        // add to result
        newMeshes.push_back(mesh);
    }
}
} // namespace

std::vector<TerrainMeshData> generateMesh(const std::vector<Block> &blocks, tg::ipos3 chunkPos)
{
    GLOW_ACTION("[WORKER] - create mesh"); // time this method (shown on shutdown)

    std::vector<int> built;                 // track already built materials
    std::vector<TerrainMeshData> newMeshes; // build new mesh list

    auto block = [&blocks](int x, int y, int z) { return blocks[(z * EXT_SIZE + y) * EXT_SIZE + x]; };

    auto hasMat = [&built](int mat) {
        for (auto m : built)
            if (m == mat)
                return true;
        return false;
    };

    // ensure that each material is accounted for
    for (auto z = 1; z <= CHUNK_SIZE; ++z)
        for (auto y = 1; y <= CHUNK_SIZE; ++y)
            for (auto x = 1; x <= CHUNK_SIZE; ++x)
            {
                auto const &b = block(x, y, z); // get block

                // if block material is not air and not already built
                if (!b.isAir() && !hasMat(b.mat))
                {
                    // mark as built
                    built.push_back(b.mat);

                    // create VAO(s)
                    buildMeshFor(blocks, chunkPos, b.mat, newMeshes);
                }
            }

    return newMeshes;
}
