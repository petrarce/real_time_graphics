#include "Scene.hh"

#include "renderables/Renderable.hh"

using namespace glow;
using namespace glow::viewer;

void Scene::add(const SharedRenderable& r) { mRenderables.push_back(r); }

aabb Scene::computeAabb() const
{
    aabb bounds;
    for (auto const& r : mRenderables)
        bounds.add(r->computeAabb());
    return bounds;
}

Scene::BoundingInfo Scene::getBoundingInfo() const
{
    auto aabb = computeAabb();
    if (aabb.isEmpty())
    {
        // Centered unit aabb for algorithm sanity down the line
        aabb.min = tg::pos3(-.5f);
        aabb.max = tg::pos3(.5f);
    }

    auto const size = aabb.size();
    return BoundingInfo{length(size), aabb.center(), tg::aabb3(aabb.min, aabb.max)};
}
