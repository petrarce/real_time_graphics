#pragma once

#include "GeometricRenderable.hh"

#include "../objects/geometry/LineBuilder.hh"

namespace glow
{
namespace viewer
{
class LineRenderable final : public GeometricRenderable
{
private:
    bool mRoundCaps = false;
    bool mNoCaps = false;
    bool mExtrapolate = false;
    bool m3D = false;
    bool mCameraFacing = false;
    bool mWorldSpaceSize = false;

    // is lazily built
    glow::SharedVertexArray mVertexArray;
    glow::SharedProgram mForwardShader;
    glow::SharedProgram mShadowShader;

public:
    aabb computeAabb() override;

    void renderShadow(RenderInfo const& info) override;
    void renderForward(RenderInfo const& info) override;

public:
    static SharedLineRenderable create(builder::LineBuilder const& builder);

private:
    void initFromBuilder(builder::LineBuilder const& builder);

    void init() override;
};
}
}
