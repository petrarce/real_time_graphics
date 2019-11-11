#include "Texture.hh"

#include <glow/glow.hh>

#include <typed-geometry/common/assert.hh>

#include <limits>

using namespace glow;

Texture::Texture(GLenum target, GLenum bindingTarget, GLenum internalFormat)
  : mTarget(target), mBindingTarget(bindingTarget), mInternalFormat(internalFormat)
{
    checkValidGLOW();

    mObjectName = std::numeric_limits<decltype(mObjectName)>::max();
    glGenTextures(1, &mObjectName);
    TG_ASSERT(mObjectName != std::numeric_limits<decltype(mObjectName)>::max() && "No OpenGL Context?");

    GLint oldTex;
    glGetIntegerv(mBindingTarget, &oldTex);
    glBindTexture(mTarget, mObjectName); // pin this texture to the correct type (and ensure it's generated)
    glBindTexture(mTarget, oldTex);      // restore prev
}

Texture::~Texture()
{
    checkValidGLOW();

    glDeleteTextures(1, &mObjectName);
}
