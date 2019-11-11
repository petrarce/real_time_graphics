#pragma once

#include <vector>

#include <typed-geometry/tg-lean.hh>

#include <glow/common/property.hh>
#include <glow/common/shared.hh>

#include "aabb.hh"

namespace glow::viewer
{
GLOW_SHARED(class, Renderable);

class Scene
{
public:
    struct BoundingInfo
    {
        float diagonal = 1.f;
        tg::pos3 center = tg::pos3::zero;
        tg::aabb3 aabb = tg::aabb3::unit_centered;
    };

    // members
private:
    std::vector<SharedRenderable> mRenderables;

    /// computes the AABB of the scene
    aabb computeAabb() const;

    // properties
public:
    GLOW_GETTER(Renderables);

    bool enableGrid = true;
    bool enablePrintMode = false;
    bool enableShadows = true;
    bool enableOutlines = true;
    bool infiniteAccumulation = false;
    bool clearAccumulation = false;
    tg::color3 bgColorInner = tg::color3(.9f);
    tg::color3 bgColorOuter = tg::color3(.6f);
    float ssaoPower = 4.f;
    float ssaoRadius = 1.f;

    bool enableTonemap = false;
    float tonemapExposure = 1.f;

    bool customCameraOrientation = false;
    tg::angle cameraAzimuth = 0_deg;
    tg::angle cameraAltitude = 0_deg;
    float cameraDistance = -1.f;

    bool customCameraPosition = false;
    tg::pos3 cameraPosition = tg::pos3(1, 1, 1);
    tg::pos3 cameraTarget = tg::pos3::zero;

    bool enableScreenshotDebug = false;

    tg::mat4 viewMatrix;
    tg::mat4 projMatrix;

    Scene() = default;

    // methods
public:
    /// adds a renderable
    void add(SharedRenderable const& r);

    BoundingInfo getBoundingInfo() const;
};
}
