#pragma once

#include <typed-geometry/tg-lean.hh>

#include <glow/common/property.hh>

namespace glow
{
namespace camera
{
/**
 * The Lens
 *
 * Component of the camera responsible for projection
 */
class Lens
{
private:
    // == Parameters ==
    tg::isize2 mViewportSize;
    float mAspectRatio = 1.f;

    tg::angle mHorizontalFov = 80_deg;

    float mNearPlane = .1f;
    float mFarPlane = 1000.f;

public:
    // == Setters ==
    void setNearPlane(float nearPlane) { mNearPlane = nearPlane; }
    void setFarPlane(float farPlane) { mFarPlane = farPlane; }
    void setFoV(tg::angle fov) { mHorizontalFov = fov; }
    void setViewportSize(int w, int h);

    // == Getters ==
    GLOW_GETTER(ViewportSize);
    GLOW_GETTER(AspectRatio);
    GLOW_GETTER(HorizontalFov);
    GLOW_GETTER(NearPlane);
    GLOW_GETTER(FarPlane);

    tg::mat4 getProjectionMatrix() const;
    tg::angle getVerticalFov() const;
};
}
}
