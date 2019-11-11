#pragma once

#include "GeometricRenderable.hh"

#include "../objects/geometry/PolygonBuilder.hh"

namespace glow
{
namespace viewer
{
class MeshRenderable final : public GeometricRenderable
{
private:
    // is lazily built
    glow::SharedVertexArray mMesh;
    glow::SharedProgram mForwardShader;
    glow::SharedProgram mShadowShader;

public:
    aabb computeAabb() override;

    void renderShadow(RenderInfo const& info) override;
    void renderForward(RenderInfo const& info) override;
    void renderTransparent(RenderInfo const& info) override;

    void init() override;

private:
    void renderMesh(RenderInfo const& info);

public:
    static SharedMeshRenderable create(builder::PolygonBuilder const& builder);
};
}
}
