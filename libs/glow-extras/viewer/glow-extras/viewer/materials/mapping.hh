#pragma once

#include "ColorMapping.hh"

namespace glow
{
namespace viewer
{
template <class T>
ColorMapping mapping(T const& data)
{
    return ColorMapping().data(data);
}
template <class T>
ColorMapping mapping(T const& data, glow::SharedTexture1D const& tex)
{
    return mapping(data).texture(tex);
}
template <class T>
ColorMapping mapping(T const& data, glow::SharedTexture2D const& tex)
{
    return mapping(data).texture(tex);
}
template <class T>
ColorMapping mapping(T const& data, glow::SharedTexture3D const& tex)
{
    return mapping(data).texture(tex);
}
}
}

namespace polymesh
{
// insert into polymesh namespace so ADL works
// e.g. view(pos, mapping(data)) works without using any namespace (pos is polymesh::vertex_attribute<glm::vec3>)
using glow::viewer::mapping;
}
