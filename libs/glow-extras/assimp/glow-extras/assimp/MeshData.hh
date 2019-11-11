#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glow/common/shared.hh>
#include <glow/fwd.hh>

namespace glow
{
namespace assimp
{
GLOW_SHARED(struct, MeshData);

struct MeshData
{
    std::string filename;

    glm::vec3 aabbMin;
    glm::vec3 aabbMax;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;

    // one vector per channel
    std::vector<std::vector<glm::vec2>> texCoords;
    std::vector<std::vector<glm::vec4>> colors;

    std::vector<uint32_t> indices;

    SharedVertexArray createVertexArray() const;
};
}
}
