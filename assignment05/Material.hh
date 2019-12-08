#pragma once

#include <string>

#include <glow/fwd.hh>

struct Material
{
    std::string name;
    int8_t index = 0; // is assigned when adding

    std::string shader;

    // material
    float metallic = 0.0;
    float reflectivity = 0.3;

    // textures
    float textureScale = 1.0f;
    glow::SharedTexture2D texAlbedo;
    glow::SharedTexture2D texAO;
    glow::SharedTexture2D texHeight;
    glow::SharedTexture2D texNormal;
    glow::SharedTexture2D texRoughness;
};
