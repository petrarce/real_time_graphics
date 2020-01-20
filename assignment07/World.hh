#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include <typed-geometry/tg-std.hh>

#include "Chunk.hh"
#include "Material.hh"
#include "helper/Noise.hh"

#include "Constants.hh"

#include "TerrainWorker.hh"

struct RayHit
{
    bool hasHit = false;

    tg::pos3 hitPos;
    tg::ivec3 hitNormal;

    Block block;
    tg::ipos3 blockPos;
};

class World
{
public: // public members
    /// list of active chunks
    std::unordered_map<tg::ipos3, SharedChunk> chunks;

    /// list of opaque materials
    std::vector<Material> materialsOpaque;
    /// list of translucent materials
    std::vector<Material> materialsTranslucent;

private: // private members
    /// Noise generator
    FastNoise mNoiseGen;

    /// List of chunks that require updating
    std::vector<Chunk*> mDirtyChunks;

    /// list of RenderMaterials
    std::vector<SharedRenderMaterial> renderMaterials;

    /// worker thread
    TerrainWorker mWorker;

public:
    World();
    ~World();

    /// initializes the world (materials, chunks, ...)
    void init();

    /// ensures that a chunk at a given position exists
    void ensureChunkAt(tg::ipos3 p);

    /// ensures that all required chunks around the camera are generated
    void notifyCameraPosition(tg::pos3 pos, float renderDistance, int maxChunksPerFrame = 1);

    /// deletes all chunks
    void clearChunks();

    /// adds a chunk to the update list
    /// also triggers mesh update
    void notifyDirtyChunk(Chunk* chunk);

    /// notifies that a chunk was generated
    void notifyChunkGenerated(SharedChunk chunk);
    /// notifies that a chunk mesh was updated
    void notifyChunkMeshed(SharedChunk chunk, std::vector<TerrainMeshData> const& data);

    /// Update step
    void update(float elapsedSeconds);

private: // helper
    /// creates all materials
    void setUpMaterials();

    /// triggers a mesh update for a given chunk
    void triggerMeshUpdate(SharedChunk chunk);

    /// Adds an opaque material, automatically searches textures
    /// CAREFUL: return value only valid until next mat is added
    Material& addOpaqueMat(std::string const& name, std::vector<SharedRenderMaterial> materials);
    /// Adds a translucent material, automatically searches textures
    /// CAREFUL: return value only valid until next mat is added
    Material& addTranslucentMat(std::string const& name, std::vector<SharedRenderMaterial> materials);
    /// Try to find default textures by name
    void addDefaultTextures(SharedRenderMaterial mat, const std::string& name);

    /// Copy/extend RenderMaterials to Material
    void copyRenderMaterials(Material& mat, std::vector<SharedRenderMaterial> const& renderMats);

    /// Performs procedural generation of a chunk
    void generate(Chunk& c);

public: // accessor functions
    /// for a given world space position, returns the starting position of the associated chunk
    tg::ipos3 chunkPos(tg::ipos3 p) const
    {
        p.x -= (p.x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
        p.y -= (p.y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
        p.z -= (p.z % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
        return p;
    }

    /// Returns the chunk that contains the given position
    /// Returns nullptr if it doesn't exist
    Chunk* queryChunk(tg::ipos3 p) const;
    /// Returns the chunk that contains the given position
    /// Allocates chunk if not existent
    Chunk& queryChunkAlloc(tg::ipos3 p);

    /// queries a block at a given position
    /// returns an INVALID block if not found
    /// (does not allocate new chunks dynamically)
    Block const& queryBlock(tg::ipos3 p) const;
    /// queries a block at a given position
    /// returns a MUTABLE reference to the block
    /// allocates chunks dynamically
    Block& queryBlockMutable(tg::ipos3 p);

    /// Marks all blocks in a given radius as dirty
    void markDirty(tg::ipos3 p, int rad);

    /// Returns the material of that idx
    /// nullptr if that material does not exists (or is air)
    Material const* getMaterialFromIndex(int matIdx) const;
    /// Returns the material of that name
    /// It is an error if that material does not exist
    Material const* getMaterialFromName(std::string const& name) const;

    /// Casts a ray into the sceen and returns true if something was hit with max distance maxRange
    /// if getFurthestAirBlock is true, it returns the air block "in front" of that block
    RayHit rayCast(tg::pos3 pos, tg::vec3 dir, float maxRange = 100.0f) const;

    friend class TerrainWorker;
};
