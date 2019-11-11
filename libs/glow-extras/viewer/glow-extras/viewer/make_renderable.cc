
#include <polymesh/formats.hh>

#include "make_renderable.hh"

namespace glow::viewer
{
SharedMeshRenderable make_renderable(builder::PolygonBuilder const& builder)
{
    auto r = MeshRenderable::create(builder);
    r->transform(builder.getTransform());
    return r;
}

SharedPointRenderable make_renderable(builder::PointBuilder const& builder)
{
    auto r = PointRenderable::create(builder);
    r->transform(builder.getTransform());
    return r;
}

SharedLineRenderable make_renderable(builder::LineBuilder const& builder)
{
    auto r = LineRenderable::create(builder);
    r->transform(builder.getTransform());
    return r;
}

SharedVector2DRenderable make_renderable(const vector::image2D& img) { return Vector2DRenderable::create(img); }

SharedGeometricRenderable make_renderable(file const& file)
{
    pm::Mesh m;
    pm::vertex_attribute<tg::pos3> pos(m);
    load(file.filepath, m, pos);
    return make_renderable(pos);
}

} // namespace viewer
