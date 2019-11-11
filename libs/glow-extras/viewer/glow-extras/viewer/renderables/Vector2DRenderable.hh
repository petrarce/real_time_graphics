#pragma once

#include "Renderable.hh"

#include <glow-extras/vector/image2D.hh>

namespace glow
{
namespace viewer
{
// TODO: move and scale image
class Vector2DRenderable final : public Renderable
{
private:
    vector::image2D mImage;

public:
    aabb computeAabb() override { return aabb::empty(); }

    void renderOverlay(vector::OGLRenderer const* oglRenderer, tg::isize2 const& res) override;

public:
    static SharedVector2DRenderable create(vector::image2D const& img);
};
}
}
