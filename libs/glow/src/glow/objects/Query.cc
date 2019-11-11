#include "Query.hh"

#include <glow/glow.hh>

#include <typed-geometry/common/assert.hh>

#include <limits>

glow::Query::Query(GLenum _defaultTarget) : mTarget(_defaultTarget)
{
    checkValidGLOW();

    mObjectName = std::numeric_limits<decltype(mObjectName)>::max();
    glGenQueries(1, &mObjectName);
    TG_ASSERT(mObjectName != std::numeric_limits<decltype(mObjectName)>::max() && "No OpenGL Context?");

    // ensure query exists
    glBeginQuery(mTarget, mObjectName);
    glEndQuery(mTarget);
}
