#pragma once

#include <glm/vec3.hpp>

namespace glow
{
namespace viewer
{
namespace builder
{
class PlaneBuilder
{
public:
    PlaneBuilder(glm::vec3 normal) : mNormal(normal) {}

    PlaneBuilder& origin(glm::vec3 pos)
    {
        mOrigin = pos;
        mRelativeToAABB = false;
        return *this;
    }
    PlaneBuilder& relativeToAABB(glm::vec3 relativePos)
    {
        mOrigin = relativePos;
        mRelativeToAABB = true;
        return *this;
    }

private:
    glm::vec3 mNormal;
    glm::vec3 mOrigin;
    bool mRelativeToAABB = true;

    // TODO: material
};
}
}
}