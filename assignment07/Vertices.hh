#pragma once

#include <vector>

#include <typed-geometry/tg-lean.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

/// Position is stored separately
struct TerrainVertex
{
    int flags;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&TerrainVertex::flags, "aFlags"}, //
        };
    }
};

struct LightVertex
{
    tg::pos3 position;
    float radius;
    tg::color3 color;
    int seed;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&LightVertex::position, "aLightPosition"}, //
            {&LightVertex::radius, "aLightRadius"},     //
            {&LightVertex::color, "aLightColor"},       //
            {&LightVertex::seed, "aLightSeed"},         //
        };
    }
};
