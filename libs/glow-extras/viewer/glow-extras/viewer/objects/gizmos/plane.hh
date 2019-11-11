#pragma once

#include "PlaneBuilder.hh"

namespace glow
{
namespace viewer
{
inline builder::PlaneBuilder plane(glm::vec3 n = {0, 1, 0})
{
    return builder::PlaneBuilder(n).relativeToAABB({n.x > 0 ? 1 : 0, n.y > 0 ? 1 : 0, n.z > 0 ? 1 : 0});
}
inline builder::PlaneBuilder plane(glm::vec3 n, glm::vec3 pos)
{
    return builder::PlaneBuilder(n).origin(pos);
}
}
}