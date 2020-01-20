#include "World.hh"

#include <fstream>

#include <glow-extras/timing/CpuTimer.hh>
#include <glow/common/log.hh>
#include <glow/common/profiling.hh>
#include <glow/common/str_utils.hh>
#include <glow/objects/Texture2D.hh>

#include <typed-geometry/tg.hh>

#include <cstdlib>
#include <cstring>

#include "helper/Noise.hh"

#include "Chunk.hh"
#include "Material.hh"

World::World() : mWorker(this) {}

World::~World()
{
    mWorker.stop();
}

void World::init()
{
    // set up materials (name / shader)
    setUpMaterials();

    // configure world gen
    mNoiseGen.SetNoiseType(FastNoise::SimplexFractal);
}

void World::setUpMaterials()
{
    // Have material m on all 6 sides of the cube
    auto all = [](SharedRenderMaterial const& m) -> std::vector<SharedRenderMaterial> {
        return {m, m, m, m, m, m}; //
    };
    // Have material t on the top side of the cube and material s everywhere else
    auto topAndSide = [](SharedRenderMaterial const& t, SharedRenderMaterial const& s) -> std::vector<SharedRenderMaterial> {
        return {s, s, s, s, t, s}; //
    };

    // Helper function to create and add a render material
    auto createRM = [this](std::string const& basename, float scale = 3.35, bool opaque = true,
                           std::string const& shader = "opaque", float reflectivity = 0.3, float metallic = 0.0f,
                           float translucency = 0.0f) -> SharedRenderMaterial {
        auto rm = std::make_shared<RenderMaterial>();
        addDefaultTextures(rm, basename);
        rm->textureScale = scale;
        rm->opaque = opaque;
        rm->shader = shader;
        rm->reflectivity = reflectivity;
        rm->metallic = metallic;
        rm->translucency = translucency;
        renderMaterials.push_back(rm);
        return rm;
    };

    // Create render materials

    auto dirtRM = createRM("mud");
    auto grassRM = createRM("grass");
    auto solarRM = createRM("solar", 1.0, true, "opaque", 0.3, 0.7, 0.7);
    auto rockRM = createRM("rock");
    auto sandRM = createRM("sand", 3.35, true, "opaque", 0.3, 0.0, 0.5);
    auto snowRM = createRM("snow", 3.35, true, "opaque", 0.3, 0.0, 1.0);
    auto goldRM = createRM("gold", 3.35, true, "opaque", 0.96, 1.0);     // hack: less metalness for more albedo
    auto copperRM = createRM("copper", 3.35, true, "opaque", 0.97, 1.0); // hack: less metalness for more albedo
    auto bronzeRM = createRM("bronze", 3.35, true, "opaque", 0.95, 1.0); // hack: less metalness for more albedo


    // TODO: fix missing triangles in cases where two terrain mats share a render mat
    auto dirtRM2 = std::make_shared<RenderMaterial>(*dirtRM);
    auto rockRM2 = std::make_shared<RenderMaterial>(*rockRM);
    auto snowRM2 = std::make_shared<RenderMaterial>(*snowRM);

    auto crystalRM = createRM("crystal", 3.35, false, "glass", 0.4);
    auto waterRM = createRM("water", 10.0, false, "water", 1.0);


    // Create terrain materials (that consist of 6 render materials each)

    addOpaqueMat("grass", topAndSide(grassRM, dirtRM2));
    addOpaqueMat("dirt", all(dirtRM));
    addOpaqueMat("lightfountain", all(solarRM)).spawnsLightSources = true;
    addOpaqueMat("sand", all(sandRM));
    addOpaqueMat("rock", all(rockRM));
    addOpaqueMat("snow", all(snowRM));
    addOpaqueMat("snowrock", topAndSide(snowRM2, rockRM2));
    addOpaqueMat("gold", all(goldRM));
    addOpaqueMat("copper", all(copperRM));
    addOpaqueMat("bronze", all(bronzeRM));

    addTranslucentMat("crystal", all(crystalRM));
    addTranslucentMat("water", all(waterRM));
}

void World::triggerMeshUpdate(SharedChunk chunk)
{
    if (!chunk->isGenerated())
        return; // not generated -> no mesh

    GLOW_ACTION();

    // build blocks
    auto cs = CHUNK_SIZE + 2;
    std::vector<Block> blocks(cs * cs * cs, Block::invalid());
    auto bmin = chunk->chunkPos - 1;
    auto bmax = chunk->chunkPos + CHUNK_SIZE + 1;
    for (auto dz : {-1, 0, 1})
        for (auto dy : {-1, 0, 1})
            for (auto dx : {-1, 0, 1})
            {
                auto cp = chunk->chunkPos + CHUNK_SIZE * tg::ivec3(dx, dy, dz);
                auto c = queryChunk(cp);
                if (!c)
                    continue;

                // copy blocks
                auto min = clamp(c->chunkPos, bmin, bmax) - c->chunkPos;
                auto max = clamp(c->chunkPos + CHUNK_SIZE, bmin, bmax) - c->chunkPos;

                auto xCount = max.x - min.x;

                for (auto z = min.z; z < max.z; ++z)
                    for (auto y = min.y; y < max.y; ++y)
                    {
                        auto ty = y + dy * CHUNK_SIZE + 1;
                        auto tz = z + dz * CHUNK_SIZE + 1;
                        auto tminx = min.x + dx * CHUNK_SIZE + 1;

                        // copy line
                        memcpy(&blocks[(tz * cs + ty) * cs + tminx], &c->block({min.x, y, z}), xCount * sizeof(Block));
                    }
            }

    // bump mesh version
    chunk->mMeshVersion++;

    // enqueue job
    mWorker.enqueueMesh(chunk, std::move(blocks));
}

void World::ensureChunkAt(tg::ipos3 p)
{
    auto cp = chunkPos(p);
    if (chunks.count(cp))
        return; // exists

    // create chunk
    auto c = Chunk::create(cp, this);

    // register chunk
    chunks[cp] = c;

    // send to worker
    mWorker.enqueueGen(c);
}

void World::notifyCameraPosition(tg::pos3 pos, float renderDistance, int maxChunksPerFrame)
{
    // spiral pattern
    for (auto dis = 0; dis < renderDistance + CHUNK_SIZE * 2; dis += CHUNK_SIZE)
        for (auto dx = -dis; dx <= dis; dx += CHUNK_SIZE)
            for (auto dz = -dis; dz <= dis; dz += CHUNK_SIZE)
                if (tg::abs(dx) == dis || tg::abs(dz) == dis)
                {
                    auto x = (int)pos.x + dx;
                    auto z = (int)pos.z + dz;

                    auto ip = tg::ipos3(x, 0, z);

                    // out of render dis
                    auto cp = chunkPos(ip);
                    auto inChunkPos = tg::clamp(pos, tg::pos3(cp), tg::pos3(cp + CHUNK_SIZE + 1));
                    if (distance(inChunkPos, pos) > renderDistance)
                        continue;

                    // only trigger one chunk, rest is done after generation
                    ensureChunkAt(ip);
                }
}

void World::clearChunks()
{
    // removes all chunks
    // due to shared_ptr's also clears all associated memory
    chunks.clear();
}

void World::notifyDirtyChunk(Chunk* chunk)
{
    // queue update
    mDirtyChunks.push_back(chunk);
}

void World::notifyChunkGenerated(SharedChunk c)
{
    // chunk is now generated
    c->mIsGenerated = true;

    // mark neighboring chunks as dirty
    for (auto dz = -1; dz <= 1; ++dz)
        for (auto dy = -1; dy <= 1; ++dy)
            for (auto dx = -1; dx <= 1; ++dx)
            {
                auto nc = queryChunk(c->chunkPos + tg::ivec3(dx, dy, dz) * CHUNK_SIZE);
                if (nc)
                    nc->markDirty();
            }

    // do CPU update
    c->update();

    // trigger gen up
    if (!c->isFullyAir())
        ensureChunkAt(c->chunkPos + tg::ivec3(0, CHUNK_SIZE, 0));

    // trigger gen down
    if (!c->isFullySolid())
        ensureChunkAt(c->chunkPos - tg::ivec3(0, CHUNK_SIZE, 0));

    // fail safe
    if (c->chunkPos.y < -500 || c->chunkPos.y > 500)
    {
        glow::info() << "DEEP OR HIGH CHUNK - memory not optimized for this";
        glow::info() << " at " << c->chunkPos;
        glow::info() << " is fully air? " << c->isFullyAir();
        glow::info() << " is fully solid? " << c->isFullySolid();
        for (auto m : c->queryMaterials())
            glow::info() << " - contains " << (m ? m->name : "<air>");
        glow::info() << " aborting before memory issues arise.";
        exit(0);
    }
}

void World::notifyChunkMeshed(SharedChunk chunk, std::vector<TerrainMeshData> const& data)
{
    chunk->notifyMeshData(data);
}

void World::update(float elapsedSeconds)
{
    // update dirty chunks
    glow::timing::CpuTimer timer;
    for (auto i = (int)mDirtyChunks.size() - 1; i >= 0; --i)
    {
        auto c = mDirtyChunks[i];

        // perform CPU update
        c->update(); // CAUTION: if update might trigger new dirty chunks, this should be guarded

        // queue mesh update
        triggerMeshUpdate(chunks[c->chunkPos]);

        // remove from list
        mDirtyChunks.erase(mDirtyChunks.begin() + i);

        if (timer.elapsedSeconds() > 5 / 1000.0f)
            break; // more than 5ms updates -> STOP
    }

    // update worker
    mWorker.update();
}

Material& World::addOpaqueMat(std::string const& name, std::vector<SharedRenderMaterial> renderMats)
{
    Material mat;
    mat.name = name;
    mat.index = materialsOpaque.size() + 1;
    copyRenderMaterials(mat, renderMats);
    materialsOpaque.push_back(mat);
    return materialsOpaque.back();
}

Material& World::addTranslucentMat(std::string const& name, std::vector<SharedRenderMaterial> renderMats)
{
    Material mat;
    mat.name = name;
    mat.index = -(materialsTranslucent.size() + 1);
    copyRenderMaterials(mat, renderMats);
    materialsTranslucent.push_back(mat);
    return materialsTranslucent.back();
}

void World::addDefaultTextures(SharedRenderMaterial mat, std::string const& name)
{
    using namespace glow;
    auto texPath = util::pathOf(__FILE__) + "/textures/terrain/";

    auto tryLoad = [&](std::string const& name, ColorSpace colorspace) -> SharedTexture2D {
        auto file = texPath + name;
        if (!std::fstream(file).good())
            return nullptr;
        info() << " - loading texture /textures/terrain/" << name;
        return Texture2D::createFromFile(file, colorspace);
    };

    mat->texAlbedo = tryLoad(name + ".albedo.png", ColorSpace::sRGB);
    mat->texAO = tryLoad(name + ".ao.png", ColorSpace::Linear);
    mat->texNormal = tryLoad(name + ".normal.png", ColorSpace::Linear);
    mat->texRoughness = tryLoad(name + ".roughness.png", ColorSpace::Linear);
    mat->texHeight = tryLoad(name + ".height.png", ColorSpace::Linear);
}

void World::copyRenderMaterials(Material& mat, const std::vector<SharedRenderMaterial>& renderMats)
{
    // TODO: helper methods 1,2,3...->6
    const size_t nRenderMats = renderMats.size();
    if (nRenderMats != 6u)
    {
        glow::error() << "Need to provide exactly 6 RenderMaterials for population of terrain material (given: " << nRenderMats
                      << ")";
        return;
    }

    for (auto i = 0; i < 6; ++i)
    {
        // unique material for each side
        mat.renderMaterials[i] = renderMats[i];
    }
}

namespace
{
uint32_t wang_hash(uint32_t seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27D4EB2Du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

float getRandFloat01Wang(tg::ivec3 relPos)
{
    uint32_t seed = ((relPos.x * 123) + relPos.y) * 5 + relPos.z;
    return float(wang_hash(seed)) / std::numeric_limits<uint32_t>::max();
}
} // namespace
void World::generate(Chunk& c)
{
    GLOW_ACTION("[WORKER] - generate chunk");

    auto matAir = nullptr;
    auto matGrass = getMaterialFromName("grass");
    auto matDirt = getMaterialFromName("dirt");
    auto matRock = getMaterialFromName("rock");
    auto matSand = getMaterialFromName("sand");
    auto matSnow = getMaterialFromName("snow");
    auto matSnowRock = getMaterialFromName("snowrock");

    auto matGold = getMaterialFromName("gold");
    auto matCopper = getMaterialFromName("copper");
    auto matBronze = getMaterialFromName("bronze");

    auto matWater = getMaterialFromName("water");
    auto matCrystal = getMaterialFromName("crystal");
    auto matLightFountain = getMaterialFromName("lightfountain");


    // TODO: cooler

    for (auto z = 0; z < CHUNK_SIZE; ++z)
        for (auto y = 0; y < CHUNK_SIZE; ++y)
            for (auto x = 0; x < CHUNK_SIZE; ++x)
            {
                auto rp = tg::ivec3(x, y, z);
                auto ip = c.chunkPos + rp;
                auto p = tg::pos3(ip);

                // terrain options
                const auto waterDepthFactor = 3.0;
                const auto hillHeightFactor = 8.0;
                const auto flatLandFactor = 0.3;
                auto seaLevel = 0;

                // generate terrain
                auto d = 25 * (mNoiseGen.GetPerlinFractal(2.0 * p.x, 2.0 * p.z) + 0.15);
                if (d < 0)
                    d *= waterDepthFactor;
                else
                    d *= tg::mix(flatLandFactor, hillHeightFactor,
                                  tg::smoothstep(0.5, 0.7, 0.5 + 0.5 * mNoiseGen.GetPerlinFractal(.17 * p.x, .18 * p.z)));

                // choose material depending on terrain height
                Material const* mat = matAir;
                if (p.y <= d)
                {
                    if (p.y < 1)
                        mat = matSand;
                    else
                    {
                        auto grassDist = 6 + 4 * mNoiseGen.GetPerlinFractal(15.17 * p.x, 17.18 * p.z);
                        if (p.y < grassDist)
                        {
                            mat = matGrass;

                            // Avoid grass below the surface
                            if (y > 0)
                            {
                                Block& below = c.block(tg::ivec3(x, y - 1, z));
                                if (below.mat == matGrass->index)
                                    below.mat = matDirt->index;
                            }
                        }
                        else
                        {
                            auto snowDist = 12 + 3 * mNoiseGen.GetPerlinFractal(5.17 * p.x, 7.18 * p.z);
                            if (p.y > snowDist)
                                mat = matSnow;
                            else if (p.y > snowDist - 3)
                            {
                                mat = matSnowRock;
                            }
                            else
                                mat = matRock;
                        }

                        if (mat != matAir)
                        {
                            // Avoid snow and gras below surface (except snow below snow)
                            if (y > 0)
                            {
                                Block& below = c.block(tg::ivec3(x, y - 1, z));

                                if (mat != matSnow && (below.mat == matSnowRock->index || below.mat == matSnow->index))
                                {
                                    // Snow inconsistency
                                    below.mat = matRock->index;
                                }
                                else if (below.mat == matGrass->index)
                                {
                                    // No grass below surface
                                    below.mat = matDirt->index;
                                }
                            }
                        }
                    }

                    // Not in flat regions or in water
                    if (p.y >= -20 && d > 3)
                    {
                        auto cd = mNoiseGen.GetValue(12.1 * p.x, 5.1 * p.y, 9.6 * p.z);
                        // Have some small chance to generate crystal
                        if (cd > 0.8)
                            mat = matCrystal;
                        else
                        {
                            bool belowIsRock = false;
                            if (y > 0) // still in this chunk
                                belowIsRock = c.block(tg::ivec3(x, y - 1, z)).mat == matRock->index;
                            else // other chunk
                                belowIsRock = queryBlock(tg::ipos3(ip.x, ip.y - 1, ip.z)).mat == matRock->index;

                            // other minerals lie mainly below hills but only on rock material
                            if (d > 8 && belowIsRock)
                            {
                                cd = mNoiseGen.GetValue(5 + 17.3 * p.x, 4 + 23.1 * p.y, -18 + 15.6 * p.z);
                                if (cd > 0.9)
                                    mat = matGold;
                                else
                                {
                                    cd = mNoiseGen.GetValue(10.0 * p.z, 9.1 * p.x, 11.0 * p.y);
                                    if (cd > 0.9)
                                        mat = matCopper;
                                    else if (-cd > 0.9)
                                        mat = matBronze;
                                }
                            }
                        }
                    }
                }

                // water plane
                if (mat == matAir)
                {
                    if (p.y <= seaLevel)
                        mat = matWater;
                    else if (p.y <= 20 && y > 0)
                    {
                        Block const& below = c.block(tg::ivec3(x, y - 1, z));
                        if (!below.isInvalid() && below.isSolid())
                        {
                            const float spawnChance = 1 / 4000.0f;
                            if (getRandFloat01Wang(rp) < spawnChance)
                                mat = matLightFountain;
                        }
                    }
                }

                // assign material
                c.block(rp).mat = mat ? mat->index : 0;
            }

    // we changed everything!
    c.markDirty();
}

Chunk* World::queryChunk(tg::ipos3 p) const
{
    auto it = chunks.find(chunkPos(p));

    if (it == chunks.end())
        return nullptr;

    return it->second.get();
}

Chunk& World::queryChunkAlloc(tg::ipos3 p)
{
    ensureChunkAt(p);
    auto c = queryChunk(p);
    TG_ASSERT(c);
    return *c;
}

const Block& World::queryBlock(tg::ipos3 p) const
{
    static Block invalid = Block::invalid();

    auto cp = chunkPos(p);
    auto it = chunks.find(cp);

    if (it == chunks.end())
        return invalid;

    return it->second->block(p - cp);
}

Block& World::queryBlockMutable(tg::ipos3 p)
{
    auto& c = queryChunkAlloc(p);
    return c.block(p - c.chunkPos);
}

void World::markDirty(tg::ipos3 p, int rad)
{
    while (rad >= 0)
    {
        for (auto dx : {-1, 1})
            for (auto dy : {-1, 1})
                for (auto dz : {-1, 1})
                {
                    auto np = p + tg::ivec3(dx, dy, dz) * rad;
                    auto& c = queryChunkAlloc(np);
                    c.markDirty();
                }

        rad -= CHUNK_SIZE;
    }
}

Material const* World::getMaterialFromIndex(int matIdx) const
{
    if (matIdx > 0 && matIdx <= (int)materialsOpaque.size())
        return &materialsOpaque[matIdx - 1];

    if (-matIdx > 0 && -matIdx <= (int)materialsTranslucent.size())
        return &materialsTranslucent[-matIdx - 1];

    return nullptr;
}

Material const* World::getMaterialFromName(std::string const& name) const
{
    for (auto const& m : materialsOpaque)
        if (m.name == name)
            return &m;

    for (auto const& m : materialsTranslucent)
        if (m.name == name)
            return &m;

    glow::error() << "Material `" << name << "' not found";
    return nullptr;
}

RayHit World::rayCast(tg::pos3 pos, tg::vec3 dir, float maxRange) const
{
    GLOW_ACTION();

    dir.x += 1e-16 * (dir.x == 0);
    dir.y += 1e-16 * (dir.y == 0);
    dir.z += 1e-16 * (dir.z == 0);

    auto idir = tg::ivec3(tg::sign(dir.x), tg::sign(dir.y), tg::sign(dir.z));
    auto nidir = (idir + 1) / 2;
    auto ipos = tg::ipos3(tg::floor(pos));
    auto chunk = queryChunk(ipos);

    RayHit hit;
    hit.block = queryBlock(ipos);

    while ((hit.block.isAir() || hit.block.isInvalid()))
    {
        auto nextPos = ipos + nidir;
        auto nextT = (tg::pos3(nextPos) - pos) / tg::comp3(dir) + 0.001f;

        // calculate next step
        auto t = 0.0f;
        if (nextT.x <= nextT.y && nextT.x <= nextT.z) // next X
        {
            hit.hitNormal = {1, 0, 0};
            t = nextT.x;
        }
        else if (nextT.y <= nextT.z) // next Y
        {
            hit.hitNormal = {0, 1, 0};
            t = nextT.y;
        }
        else // next Z
        {
            hit.hitNormal = {0, 0, 1};
            t = nextT.z;
        }

        // advance
        pos += t * dir;
        ipos = tg::ipos3(tg::floor(pos));
        maxRange -= t;

        if (maxRange < 0)
            break;

        // update chunk
        if (chunk == nullptr || !chunk->contains(ipos))
            chunk = queryChunk(ipos);

        // update block
        if (chunk == nullptr)
            hit.block = Block::air();
        else
            hit.block = chunk->block(ipos - chunk->chunkPos);
    }

    // fill in hit
    hit.hasHit = !hit.block.isAir();
    hit.hitPos = pos;
    hit.blockPos = ipos;
    hit.hitNormal *= tg::icomp3(-idir); // correct orientation

    return hit;
}
