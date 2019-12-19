#pragma once

#include <map>
#include <vector>

#include <typed-geometry/tg.hh>

#include <glow/fwd.hh>

#include "Block.hh"

GLOW_SHARED(class, Chunk);
class World;
class Chunk
{
public: // properties
    /// chunk start position in [m]
    /// chunk goes from chunkPos .. chunkPos + size-1
    /// a block at (x,y,z) goes from (x,y,z)..(x+1,y+1,z+1)
    const tg::ipos3 chunkPos;

    /// chunk size in x,y,z dir (i.e. number of blocks per side)
    /// each block is 1m x 1m x 1m
    const int size;

    /// Backreference to the world
    World* const world = nullptr;

    /// returns true iff mesh is outdated
    bool isDirty() const { return mIsDirty; }

    /// returns the world space center of this chunk
    tg::pos3 chunkCenter() const { return tg::pos3(chunkPos) + tg::comp3(size / 2.0f); }

private: // private members
    /// List of blocks
    /// Use block(...) functions!
    std::vector<Block> mBlocks;

    /// This chunk's configured meshes
    /// Map is from material ID to vertex array
    std::map<int, glow::SharedVertexArray> mMeshes;

    /// if true, the list of blocks has changed and the mesh might be invalid
    bool mIsDirty = true;

private: // ctor
    Chunk(tg::ipos3 chunkPos, int size, World* world);

public: // create
    static SharedChunk create(tg::ipos3 chunkPos, int size, World* world);

public: // gfx
    /// returns an up-to-date version of the current meshes
    /// (might lazily rebuild the mesh)
    /// there is one mesh for each material
    std::map<int, glow::SharedVertexArray> queryMeshes();

private: // gfx helper
/// All Tasks
///
/// You can use this space for declarations of helper functions
///
/// ============= STUDENT CODE BEGIN =============

    int getAOtype(const tg::ipos3& pt, const tg::ivec3& n, const tg::ivec3& tg1, const tg::ivec3& tg2) const;
    /// Builds the mesh for a given material
    glow::SharedVertexArray buildMeshFor(int mat) const;
    /// Returns the ambient occlusion at a given position
    float aoAt(tg::ipos3 pos, tg::ivec3 dx, tg::ivec3 dy) const;

/// ============= STUDENT CODE END =============

public: // modification funcs
    /// Marks this chunk as "dirty" (triggers rebuild of mesh)
    void markDirty();

public: // accessor functions
    /// relative coordinates 0..size-1
    /// do not call outside that range
    /// relative positions are actually vectors (as they are relative to the global chunk positions)
    Block& block(tg::ivec3 relPos) { return mBlocks[(relPos.z * size + relPos.y) * size + relPos.x]; }
    Block const& block(tg::ivec3 relPos) const { return mBlocks[(relPos.z * size + relPos.y) * size + relPos.x]; }

    /// returns true iff these global coordinates are contained in this block
    bool contains(tg::ipos3 p) const
    {
        return chunkPos.x <= p.x && p.x < chunkPos.x + size && //
               chunkPos.y <= p.y && p.y < chunkPos.y + size && //
               chunkPos.z <= p.z && p.z < chunkPos.z + size;
    }

    /// queries a block in global coordinates
    /// will first search locally and otherwise consult world
    Block const& queryBlock(tg::ipos3 worldPos) const;
};
