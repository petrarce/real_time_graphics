#pragma once

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <glow/common/shared.hh>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include "Cube.hh"

namespace glow
{
namespace geometry
{
/// Similar to geometry::Cube, but evenly textured,
/// and extending from the origin in positive axis directions
///
/// Can be used like Geometry Brushes in Hammer SDK or UE4
template <typename VertexT = CubeVertex>
class CubeBrush
{
public:
    /// List of cube attributes
    std::vector<ArrayBufferAttribute> attributes;

    /// Size of the cubebrush
    glm::vec3 size;

    /// In percent, how much of the texture is used per (size) unit
    float textureScale;

public: // default vertex creator
    static VertexT createVertex(glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoord)
    {
        return {position, normal, tangent, texCoord};
    }

public:
    CubeBrush(glm::vec3 size, float texScale = 1, std::vector<ArrayBufferAttribute> const& attrs = attributesOf(static_cast<VertexT*>(nullptr)))
      : attributes(attrs), size(size), textureScale(texScale)
    {
    }
    /**
     * @brief generates a Vertex Array with a single Array Buffer containing quad data
     *
     * Vertex layout:
     *   VertexT{0, 0}
     *   VertexT{0, 1}
     *   VertexT{1, 0}
     *   VertexT{1, 1}
     */
    template <typename VertexCreator = decltype(createVertex)>
    SharedVertexArray generate(VertexCreator&& gen = createVertex) const
    {
        auto ab = ArrayBuffer::create();
        ab->setObjectLabel("Cube");
        auto eab = ElementArrayBuffer::create();
        eab->setObjectLabel("Cube");
        ab->defineAttributes(attributes);

        auto vi = 0u;
        auto ii = 0u;

        VertexT vertices[4 * 3 * 2];
        uint8_t indices[6 * 3 * 2];

        for (int i = 0; i < 3; ++i)
        {
            for (int s = 0; s < 2; ++s)
            {
                glm::vec3 n{i == 0, i == 1, i == 2};
                n *= s * 2 - 1;

                auto mid = n;
                auto left = glm::abs(dot(n, glm::vec3(0, 1, 0))) < .2 ? cross(glm::vec3(0, 1, 0), n) : cross(glm::vec3(0, 0, 1), n);
                auto top = cross(left, n);

                auto baseIndex = vi;

                float texCoordX = ((i == 0) ? size.z : size.x) * textureScale;
                float texCoordY = ((i == 1) ? size.z : size.y) * textureScale;

                vertices[vi++] = gen(mix(glm::vec3(0), size, (mid - left - top) * .5f + .5f), n, left, {0, 0});
                vertices[vi++] = gen(mix(glm::vec3(0), size, (mid + left - top) * .5f + .5f), n, left, {texCoordX, 0});
                vertices[vi++] = gen(mix(glm::vec3(0), size, (mid + left + top) * .5f + .5f), n, left, {texCoordX, texCoordY});
                vertices[vi++] = gen(mix(glm::vec3(0), size, (mid - left + top) * .5f + .5f), n, left, {0, texCoordY});

                indices[ii++] = baseIndex + 0;
                indices[ii++] = baseIndex + 3;
                indices[ii++] = baseIndex + 1;

                indices[ii++] = baseIndex + 3;
                indices[ii++] = baseIndex + 2;
                indices[ii++] = baseIndex + 1;
            }
        }

        ab->bind().setData(vertices);
        eab->bind().setIndices(indices);
        auto va = VertexArray::create(ab, eab, GL_TRIANGLES);
        va->setObjectLabel("Cube");
        return va;
    }

public: // Predefined attributes
    static std::vector<ArrayBufferAttribute> attributesOf(void*) { return VertexT::attributes(); }
};
}
}
