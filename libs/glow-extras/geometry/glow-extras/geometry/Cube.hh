#pragma once

#include <typed-geometry/tg.hh>

#include <glow/common/shared.hh>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

namespace glow
{
namespace geometry
{
/// Default type for Cube vertices
struct CubeVertex
{
    tg::pos3 pos;
    tg::vec3 normal;
    tg::vec3 tangent;
    tg::pos2 texCoord;

    static std::vector<ArrayBufferAttribute> attributes()
    {
        return {
            {&CubeVertex::pos, "aPosition"},    //
            {&CubeVertex::normal, "aNormal"},   //
            {&CubeVertex::tangent, "aTangent"}, //
            {&CubeVertex::texCoord, "aTexCoord"},
        };
    }
};

/// Builder for a an indexed cube
/// VertexT needs a c'tor that can be called with
///   (glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoord)
///
/// Supported built-in types:
///   CubeVertex
///
/// If you use your custom vertex struct you have 2 options:
///   A: specify all attributes in Cube(...) ctor
///   B: implemented the following function in your VertexT:
///      static std::vector<ArrayBufferAttribute> attributes();
template <typename VertexT = CubeVertex>
class Cube
{
public:
    /// List of cube attributes
    std::vector<ArrayBufferAttribute> attributes;

    /// Minimal coordinate
    tg::pos3 minCoord;
    /// Maximal coordinate
    tg::pos3 maxCoord;

public: // default vertex creator
    static VertexT createVertex(tg::pos3 position, tg::vec3 normal, tg::vec3 tangent, tg::pos2 texCoord)
    {
        return {position, normal, tangent, texCoord};
    }

public:
    Cube(std::vector<ArrayBufferAttribute> const& attrs = attributesOf(static_cast<VertexT*>(nullptr)),
         tg::pos3 minCoord = {-1, -1, -1},
         tg::pos3 maxCoord = {+1, +1, +1})
      : attributes(attrs), minCoord(minCoord), maxCoord(maxCoord)
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
                auto n = tg::vec3(i == 0, i == 1, i == 2);
                n *= s * 2 - 1;

                auto mid = n;
                auto left = tg::abs(dot(n, tg::vec3(0, 1, 0))) < .2 ? cross(tg::vec3(0, 1, 0), n) : cross(tg::vec3(0, 0, 1), n);
                auto top = cross(left, n);

                auto baseIndex = vi;

                vertices[vi++] = gen(mix(minCoord, maxCoord, tg::comp3(mid - left - top) * .5f + .5f), n, left, {0, 0});
                vertices[vi++] = gen(mix(minCoord, maxCoord, tg::comp3(mid + left - top) * .5f + .5f), n, left, {1, 0});
                vertices[vi++] = gen(mix(minCoord, maxCoord, tg::comp3(mid + left + top) * .5f + .5f), n, left, {1, 1});
                vertices[vi++] = gen(mix(minCoord, maxCoord, tg::comp3(mid - left + top) * .5f + .5f), n, left, {0, 1});

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

inline SharedVertexArray make_cube()
{
    return Cube<>().generate();
}
}
}
