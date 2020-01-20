#pragma once

#include <string>
#include <vector>

#include <glow/fwd.hh>


GLOW_SHARED(struct, RenderMaterial);

struct RenderMaterial
{
    std::string shader;

    // material
    float metallic = 0.0;
    float reflectivity = 0.3;
    float translucency = 0.0;

    // textures
    float textureScale = 1.0f;
    glow::SharedTexture2D texAlbedo;
    glow::SharedTexture2D texAO;
    glow::SharedTexture2D texHeight;
    glow::SharedTexture2D texNormal;
    glow::SharedTexture2D texRoughness;

    bool opaque = true;
};

struct Material
{
    std::string name;

    int8_t index = 0; // is assigned when adding

    // Six materials, one for each side
    // Order convention -(1,0,0), -(0,1,0), -(0,0,1), (1,0,0), (0,1,0, (0,0,1)
    // i.e: (i % 3 == 0, i % 3 == 1, i % 3 == 2) * (i < 3 ? -1 : 1)
    SharedRenderMaterial renderMaterials[6];

    // spawning light sources
    bool spawnsLightSources = false;
};
