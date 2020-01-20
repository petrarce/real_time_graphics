#pragma once

#include <glow/fwd.hh>

#include <typed-geometry/tg-lean.hh>

#include "Material.hh"
#include "Vertices.hh"

/// A terrain mesh for a (chunk, material, direction) combination
struct TerrainMesh
{
    /// The material used for this mesh
    SharedRenderMaterial mat = nullptr;

    /// Face Direction of this mesh
    tg::ivec3 dir;

    /// Bounding box
    tg::pos3 aabbMin;
    tg::pos3 aabbMax;

    /// configured geometry
    glow::SharedVertexArray vaoFull;
    glow::SharedVertexArray vaoPosOnly;

    /// vertex data
    glow::SharedArrayBuffer abPositions;
    glow::SharedArrayBuffer abData;
};

struct TerrainMeshData
{
    /// The material used for this mesh
    int8_t mat;

    /// Face Direction of this mesh
    tg::ivec3 dir;

    /// Bounding box
    tg::pos3 aabbMin;
    tg::pos3 aabbMax;

    /// Vertices
    std::vector<tg::pos3> vertexPositions;
    std::vector<TerrainVertex> vertexData;
};
