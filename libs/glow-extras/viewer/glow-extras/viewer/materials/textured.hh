#pragma once

#include "Texturing.hh"

#include <polymesh/Mesh.hh>

namespace glow
{
namespace viewer
{
inline Texturing textured(polymesh::vertex_attribute<glm::vec2> const& uv, glow::SharedTexture const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
inline Texturing textured(polymesh::halfedge_attribute<glm::vec2> const& uv, glow::SharedTexture const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
inline Texturing textured(polymesh::vertex_attribute<tg::vec2> const& uv, glow::SharedTexture const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
inline Texturing textured(polymesh::halfedge_attribute<tg::vec2> const& uv, glow::SharedTexture const& tex)
{
    return Texturing().texture(tex).coords(uv);
}

template <class T>
Texturing textured(polymesh::vertex_attribute<glm::vec2> const& uv, glow::AsyncTexture<T> const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
template <class T>
Texturing textured(polymesh::halfedge_attribute<glm::vec2> const& uv, glow::AsyncTexture<T> const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
template <class T>
Texturing textured(polymesh::vertex_attribute<tg::vec2> const& uv, glow::AsyncTexture<T> const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
template <class T>
Texturing textured(polymesh::halfedge_attribute<tg::vec2> const& uv, glow::AsyncTexture<T> const& tex)
{
    return Texturing().texture(tex).coords(uv);
}
}
}

namespace polymesh
{
// insert into polymesh namespace so ADL works
// e.g. view(pos, textured(uv, tex)) works without using any namespace (pos is polymesh::vertex_attribute<glm::vec3>)
using glow::viewer::textured;
}
