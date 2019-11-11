#pragma once

#include <glow/common/shared.hh>

namespace glow
{
namespace viewer
{
namespace detail
{
struct raii_view_closer;
GLOW_SHARED(class, MeshAttribute);
GLOW_SHARED(class, MeshDefinition);
class MeshShaderBuilder;
}

GLOW_SHARED(class, Scene);

GLOW_SHARED(class, Renderable);
GLOW_SHARED(class, GeometricRenderable);
GLOW_SHARED(class, MeshRenderable);
GLOW_SHARED(class, PointRenderable);
GLOW_SHARED(class, LineRenderable);
GLOW_SHARED(class, Vector2DRenderable);

class ColorMapping;
class Texturing;
class ViewerApp;
}

namespace colors
{
struct color;
}
}

namespace polymesh
{
class Mesh;
template <class AttrT>
struct vertex_attribute;
template <class AttrT>
struct face_attribute;
template <class AttrT>
struct edge_attribute;
template <class AttrT>
struct halfedge_attribute;
}
