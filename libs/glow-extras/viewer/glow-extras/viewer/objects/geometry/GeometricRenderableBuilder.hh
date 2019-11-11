#pragma once

#include "../../detail/MeshAttribute.hh"

namespace glow
{
namespace viewer
{
namespace builder
{
template <class ThisT>
class GeometricRenderableBuilder
{
    // transformations
public:
    ThisT& transform(tg::mat4 const& m)
    {
        mTransform = mTransform * m;
        return static_cast<ThisT&>(*this);
    }
    ThisT& move(tg::vec3 v) { return transform(tg::translation(v)); }
    ThisT& scale(tg::vec3 v) { return transform(tg::scaling(tg::size3(v))); }
    ThisT& rotate(float angle, glm::vec3 axis) { return transform(glm::rotate(angle, axis)); }
    ThisT& rotateX(float angle) { return rotate(angle, {1, 0, 0}); }
    ThisT& rotateY(float angle) { return rotate(angle, {0, 1, 0}); }
    ThisT& rotateZ(float angle) { return rotate(angle, {0, 0, 1}); }

protected:
    tg::mat4 mTransform = tg::mat4::identity;
    std::vector<detail::SharedMeshAttribute> mAttributes;

    void addAttribute(detail::SharedMeshAttribute attr)
    {
        for (auto const& a : mAttributes)
            if (a->name() == attr->name())
                throw std::logic_error("Duplicated attribute " + a->name());

        mAttributes.emplace_back(std::move(attr));
    }

public:
    GLOW_GETTER(Transform);
    GLOW_GETTER(Attributes);
};
}
}
}
