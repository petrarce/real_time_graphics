#pragma once

#include <functional>

#include <typed-geometry/tg-lean.hh>

#include <glow/fwd.hh>

#include <glow/common/property.hh>
#include <glow/common/shared.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/vector/fwd.hh>

#include "../aabb.hh"
#include "../fwd.hh"

namespace glow
{
namespace viewer
{
struct RenderInfo;

class Renderable : public std::enable_shared_from_this<Renderable>
{
    // member
private:
    tg::mat4 mTransform;
    bool mInitialized = false;
    std::string mName;

    // properties
public:
    GLOW_BUILDER(transform, Transform);
    GLOW_BUILDER(name, Name);

    // methods
public:
    virtual void renderShadow(RenderInfo const& /*info*/) {}
    virtual void renderForward(RenderInfo const& /*info*/) {}
    virtual void renderTransparent(RenderInfo const& /*info*/) {}
    virtual void renderOverlay(vector::OGLRenderer const* /*oglRenderer*/, tg::isize2 const& /*res*/) {}

    // NOTE: glow::viewer::interactive possibly obsoletes this feature,
    // but the performance overhead is low
    virtual void onGui() {}

    virtual void init() {}

    /// computes the AABB (update min/max)
    virtual aabb computeAabb() = 0;

    /// Calls virtual init once
    void runLazyInit()
    {
        if (!mInitialized)
        {
            init();
            mInitialized = true;
        }
    }

    // ctor
public:
    virtual ~Renderable() {}
};
}
}
