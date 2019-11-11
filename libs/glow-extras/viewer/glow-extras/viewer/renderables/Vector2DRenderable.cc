#include "Vector2DRenderable.hh"

#include <glow-extras/vector/backend/opengl.hh>

#include "../RenderInfo.hh"

void glow::viewer::Vector2DRenderable::renderOverlay(vector::OGLRenderer const* oglRenderer, tg::isize2 const& res)
{
    oglRenderer->render(mImage, res.width, res.height);
}

glow::viewer::SharedVector2DRenderable glow::viewer::Vector2DRenderable::create(const glow::vector::image2D& img)
{
    auto r = std::make_shared<Vector2DRenderable>();
    r->mImage = img; // copy
    return r;
}
