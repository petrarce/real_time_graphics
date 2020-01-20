#pragma once

#include <vector>

#include "TerrainMesh.hh"
#include "Block.hh"
#include <typed-geometry/tg-lean.hh>

/// Generates mesh data for a given array of blocks
/// Blocks contain 1 neighborhood
std::vector<TerrainMeshData> generateMesh(std::vector<Block> const& blocks, tg::ipos3 chunkPos);
