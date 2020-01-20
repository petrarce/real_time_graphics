#pragma once

#include <map>
#include <vector>

#include <typed-geometry/tg.hh>

#include <glow/fwd.hh>

#include "Block.hh"
#include "Constants.hh"
#include "TerrainMesh.hh"

GLOW_SHARED(class, Chunk);
class World;

/// chunk size in x,y,z dir (i.e. number of blocks per side)
/// each block is 1m x 1m x 1m
/// size is stored in CHUNK_SIZE
class Chunk
{
public: // properties
    /// chunk start position in [m]
    /// chunk goes from chunkPos .. chunkPos + size-1
    /// a block at (x,y,z) goes from (x,y,z)..(x+1,y+1,z+1)
    const tg::ipos3 chunkPos;

    /// Backreference to the world
    World* const world = nullptr;

    /// returns true iff other properties are outdated
    bool isCpuDirty() const { return mIsDirty; }

    /// true iff the chunk is solid-only
    /// (updated in queryMeshes)
    bool isFullySolid() const { return mIsFullySolid; }
    /// true iff the chunk is air-only
    /// (updated in queryMeshes)
    bool isFullyAir() const { return mIsFullyAir; }

    /// true iff chunk is fully generated
    bool isGenerated() const { return mIsGenerated; }

    /// returns mesh version nr
    int getMeshVersion() const { return mMeshVersion; }

    /// Bounding box
    tg::pos3 getAabbMin() const { return mAabbMin; }
    tg::pos3 getAabbMax() const { return mAabbMax; }

    /// returns the world space center of this chunk
    tg::pos3 chunkCenter() const { return tg::pos3(chunkPos) + CHUNK_SIZE / 2.0f; }

private: // private members
    /// List of blocks
    /// Use block(...) functions!
    std::vector<Block> mBlocks;

    /// This chunk's configured meshes
    std::vector<TerrainMesh> mMeshes;

    /// if true, the list of blocks has changed and the mesh might be invalid
    bool mIsDirty = false; //< on cpu side (updated every frame)

    /// true iff the chunk is solid-only
    bool mIsFullySolid = false;
    /// true iff the chunk is air-only
    bool mIsFullyAir = false;

    /// true iff chunk is generated
    bool mIsGenerated = false;

    /// versioning of the mesh (is incremented whenever a mesh update is triggered)
    int mMeshVersion = 0;

    /// bounding box
    tg::pos3 mAabbMin;
    tg::pos3 mAabbMax;

    /// list of blocks that spawn light sources
    std::vector<tg::ipos3> mActiveLightFountains;

private: // ctor
    Chunk(tg::ipos3 chunkPos, World* world);

    Chunk(Chunk const&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator=(Chunk const&) = delete;
    Chunk& operator=(Chunk&&) = delete;

public: // create
    static SharedChunk create(tg::ipos3 chunkPos, World* world);

public: // gfx
    /// returns an up-to-date version of the current meshes
    /// (might lazily rebuild the mesh)
    /// there is one or more meshes for each material
    std::vector<TerrainMesh> const& queryMeshes();

public: // modification funcs
    /// Marks this chunk as "dirty" (triggers rebuild of mesh)
    void markDirty();

    /// Updates cpu part of the chunk
    void update();

    /// Replaces current mesh data by new one
    void notifyMeshData(const std::vector<TerrainMeshData>& meshData);

public: // accessor functions
    /// relative coordinates 0..size-1
    /// do not call outside that range
    Block& block(tg::ivec3 relPos) { return mBlocks[(relPos.z * CHUNK_SIZE + relPos.y) * CHUNK_SIZE + relPos.x]; }
    Block const& block(tg::ivec3 relPos) const
    {
        return mBlocks[(relPos.z * CHUNK_SIZE + relPos.y) * CHUNK_SIZE + relPos.x];
    }

    /// returns true iff these global coordinates are contained in this block
    bool contains(tg::ipos3 p) const
    {
        return chunkPos.x <= p.x && p.x < chunkPos.x + CHUNK_SIZE && //
               chunkPos.y <= p.y && p.y < chunkPos.y + CHUNK_SIZE && //
               chunkPos.z <= p.z && p.z < chunkPos.z + CHUNK_SIZE;
    }

    /// queries a block in global coordinates
    /// will first search locally and otherwise consult world
    Block const& queryBlock(tg::ipos3 worldPos) const;

    /// returns a list of all materials in this chunk
    /// may contain nullptr for air!
    std::vector<Material const*> queryMaterials() const;

    /// returns a list of all active light fountains
    std::vector<tg::ipos3> const& getActiveLightFountains() const { return mActiveLightFountains; }

    friend class World;
};
