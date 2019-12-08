#include "World.hh"

#include <fstream>

#include <glow/objects/Texture2D.hh>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>
#include <glow/common/str_utils.hh>

#include <cstdlib>

#include "helper/Noise.hh"

#include "Chunk.hh"

void World::init()
{
    // set up materials (name / shader)
    setUpMaterials();

    // configure world gen
    noiseGen.SetNoiseType(FastNoise::SimplexFractal);
}

void World::setUpMaterials()
{
    // material (name / shader)

    // grass
    {
        auto& mat = addOpaqueMat("grass");
        mat.textureScale = 3.35f;
    }

    // rock
    {
        auto& mat = addOpaqueMat("rock");
        mat.textureScale = 3.35f;
        mat.metallic = 0.2f;
    }

    // sand
    {
        auto& mat = addOpaqueMat("sand");
        mat.textureScale = 3.35f;
    }

    // snow
    {
        auto& mat = addOpaqueMat("snow");
        mat.textureScale = 3.35f;
    }

    // water
    {
        auto& mat = addTranslucentMat("water");
        mat.shader = "water";
        mat.textureScale = 10.0f;
    }
}

void World::ensureChunkAt(tg::ipos3 p)
{
    auto cp = chunkPos(p);
    if (chunks.count(cp))
        return; // exists

    // create chunk
    auto c = Chunk::create(cp, chunkSize, this);

    // register chunk
    chunks[cp] = c;

    // generate/fill chunk
    generate(*c);

    // mark neighboring chunks as dirty
    for (auto dz = -1; dz <= 1; ++dz)
        for (auto dy = -1; dy <= 1; ++dy)
            for (auto dx = -1; dx <= 1; ++dx)
            {
                auto nc = queryChunk(c->chunkPos + tg::ivec3(dx, dy, dz) * chunkSize);
                if (nc)
                    nc->markDirty();
            }
}

void World::clearChunks()
{
    // removes all chunks
    // due to shared_ptr's also clears all associated memory
    chunks.clear();
}

Material& World::addOpaqueMat(std::string const& name)
{
    Material mat;
    mat.name = name;
    mat.index = materialsOpaque.size() + 1;
    mat.shader = "opaque";
    addDefaultTextures(mat);
    materialsOpaque.push_back(mat);
    return materialsOpaque.back();
}

Material& World::addTranslucentMat(std::string const& name)
{
    Material mat;
    mat.name = name;
    mat.index = -(materialsTranslucent.size() + 1);
    mat.shader = "translucent";
    addDefaultTextures(mat);
    materialsTranslucent.push_back(mat);
    return materialsTranslucent.back();
}

void World::addDefaultTextures(Material& mat)
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

    mat.texAlbedo = tryLoad(mat.name + ".albedo.png", ColorSpace::sRGB);
    mat.texAO = tryLoad(mat.name + ".ao.png", ColorSpace::Linear);
    mat.texNormal = tryLoad(mat.name + ".normal.png", ColorSpace::Linear);
    mat.texRoughness = tryLoad(mat.name + ".roughness.png", ColorSpace::Linear);
    mat.texHeight = tryLoad(mat.name + ".height.png", ColorSpace::Linear);
}

void World::generate(Chunk& c)
{
    GLOW_ACTION();

    auto matAir = nullptr;
    auto matGrass = getMaterialFromName("grass");
    auto matRock = getMaterialFromName("rock");
    auto matSand = getMaterialFromName("sand");
    auto matSnow = getMaterialFromName("snow");
    auto matWater = getMaterialFromName("water");


    // TODO: cooler

    for (auto z = 0; z < chunkSize; ++z)
        for (auto y = 0; y < chunkSize; ++y)
            for (auto x = 0; x < chunkSize; ++x)
            {
                auto rp = tg::ivec3(x, y, z);
                auto ip = c.chunkPos + rp;
                auto p = tg::pos3(ip);

                // terrain options
                const auto waterDepthFactor = 3.0f;
                const auto hillHeightFactor = 8.0f;
                const auto flatLandFactor = 0.3f;
                auto seaLevel = 0.f;

                // generate terrain
                auto d = 25 * (noiseGen.GetPerlinFractal(2 * p.x, 2 * p.z) + 0.15);
                if (d < 0)
                    d *= waterDepthFactor;
                else
                    d *= tg::mix(flatLandFactor, hillHeightFactor,
                                 tg::smoothstep(0.5f, 0.7f, 0.5f + 0.5f * noiseGen.GetPerlinFractal(.17f * p.x, .18f * p.z)));

                // choose material depending on terrain height
                Material const* mat = matAir;
                if (p.y <= d)
                {
                    if (p.y < 1)
                        mat = matSand;
                    else if (p.y < 6 + 4 * noiseGen.GetPerlinFractal(15.17 * p.x, 17.18 * p.z))
                        mat = matGrass;
                    else if (p.y > 10 + 5 * noiseGen.GetPerlinFractal(5.17 * p.x, 7.18 * p.z))
                        mat = matSnow;
                    else
                        mat = matRock;
                }

                // water plane
                if (mat == matAir && p.y <= seaLevel)
                    mat = matWater;

                // assign material
                c.block(rp).mat = mat ? mat->index : 0;
            }
}

Chunk* World::queryChunk(tg::ipos3 p) const
{
    auto it = chunks.find(chunkPos(p));

    if (it == chunks.end())
        return nullptr;

    return it->second.get();
}

const Block& World::queryBlock(tg::ipos3 p) const
{
    static Block air = Block::air();

    auto cp = chunkPos(p);
    auto it = chunks.find(cp);

    if (it == chunks.end())
        return air;

    return it->second->block(p - cp);
}

Block& World::queryBlockMutable(tg::ipos3 p)
{
    ensureChunkAt(p);

    auto c = queryChunk(p);
    TG_ASSERT(c && "should be allocated");

    return c->block(p - c->chunkPos);
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
                    ensureChunkAt(np);
                    auto c = queryChunk(np);
                    TG_ASSERT(c && "should be allocated");
                    c->markDirty();
                }

        rad -= chunkSize;
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

    auto i0 = int(tg::sign(dir.x));
    auto i1 = int(tg::sign(dir.y));
    auto i2 = int(tg::sign(dir.z));
    auto idir = tg::ivec3(i0, i1, i2);
    auto nidir = (idir + 1) / 2;
    auto ipos = tg::ipos3(tg::floor(pos));
    auto chunk = queryChunk(ipos);

    RayHit hit;
    hit.block = queryBlock(ipos);

    while (hit.block.isAir() && maxRange > 0.0f)
    {
        auto nextPos = ipos + nidir;
        auto nextT = tg::comp3(tg::pos3(nextPos) - pos) / dir + 0.001f;

        // calculate next step
        auto t = 0.0f;
        if (nextT.x <= nextT.y && nextT.x <= nextT.z) // next X
        {
            hit.hitNormal = {1, 0, 0};
            t = nextT.x;
            ipos.x += idir.x;
        }
        else if (nextT.y <= nextT.z) // next Y
        {
            hit.hitNormal = {0, 1, 0};
            t = nextT.y;
            ipos.y += idir.y;
        }
        else // next Z
        {
            hit.hitNormal = {0, 0, 1};
            t = nextT.z;
            ipos.z += idir.z;
        }

        // update chunk
        if (chunk == nullptr || !chunk->contains(ipos))
            chunk = queryChunk(ipos);

        // update block
        if (chunk == nullptr)
            hit.block = Block::air();
        else
            hit.block = chunk->block(ipos - chunk->chunkPos);

        // advance
        pos += t * dir;
        maxRange -= t;
    }

    // fill in hit
    hit.hasHit = !hit.block.isAir();
    hit.hitPos = pos;
    hit.blockPos = ipos;
    hit.hitNormal *= tg::icomp3(-idir); // correct orientation

    return hit;
}
