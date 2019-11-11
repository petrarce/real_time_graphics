// This file is auto-generated and should not be modified directly.
#include "Texture1D.hh"

#include <typed-geometry/common/assert.hh>
#include <typed-geometry/common/scalar_math.hh>
#include <typed-geometry/functions/minmax.hh>

#include <glow/data/SurfaceData.hh>
#include <glow/data/TextureData.hh>

#include <glow/glow.hh>
#include <glow/limits.hh>
#include <glow/common/runtime_assert.hh>
#include <glow/common/ogl_typeinfo.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/thread_local.hh>

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL Texture1D::BoundTexture1D *sCurrentTexture = nullptr;

Texture1D::BoundTexture1D *Texture1D::getCurrentTexture()
{
    return sCurrentTexture;
}

bool Texture1D::hasMipmapsEnabled() const
{
    switch (mMinFilter)
    {
    case GL_LINEAR_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_NEAREST_MIPMAP_NEAREST:
        return true;

    default:
        return false;
    }
}

GLenum Texture1D::getUniformType() const
{
    auto fmt = getInternalFormat();

    if (isSignedIntegerInternalFormat(fmt))
        return GL_INT_SAMPLER_1D;
    else if (isUnsignedIntegerInternalFormat(fmt))
        return GL_UNSIGNED_INT_SAMPLER_1D;
    else
        return GL_SAMPLER_1D;
}

Texture1D::Texture1D (GLenum internalFormat)
  : Texture(GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D, internalFormat)
{
}

Texture1D::Texture1D (Texture1D::Shape const& shape)
  : Texture(GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D, shape.format)
{
    bind().resize(shape.size);
}

SharedTexture1D Texture1D::create(int width, GLenum internalFormat)
{
    auto tex = std::make_shared<Texture1D>(internalFormat);
    tex->bind().resize(width);
    return tex;
}

SharedTexture1D Texture1D::create(Texture1D::Shape const& shape)
{
    auto tex = std::make_shared<Texture1D>(shape);
    return tex;
}

SharedTexture1D Texture1D::createStorageImmutable(int width, GLenum internalFormat, int mipmapLevels)
{
    auto tex = std::make_shared<Texture1D>(internalFormat);
    tex->bind().makeStorageImmutable(width, internalFormat, mipmapLevels);
    return tex;
}

SharedTexture1D Texture1D::createStorageImmutable(Shape const& shape)
{
    GLOW_RUNTIME_ASSERT(shape.immutableMipmaps >= 0, "Attemped to create Texture1D with negative amount of Mipmaps", return nullptr);
    auto tex = std::make_shared<Texture1D>(shape);
    tex->bind().makeStorageImmutable(shape.size, shape.format, shape.immutableMipmaps);
    return tex;
}

SharedTexture1D Texture1D::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace));
    t->setObjectLabel(filename);
    return t;
}

SharedTexture1D Texture1D::createFromFile(const std::string &filename, GLenum internalFormat, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace), internalFormat);
    t->setObjectLabel(filename);
    return t;
}

SharedTexture1D Texture1D::createFromData(const SharedTextureData &data)
{
    if (!data)
    {
        error() << "Texture1D::createFromData failed, no data provided";
        return nullptr;
    }

    if (data->getPreferredInternalFormat() == GL_INVALID_ENUM)
    {
        error() << "Texture1D::createFromData failed, no preferred internal format specified";
        return nullptr;
    }

    auto tex = std::make_shared<Texture1D>(data->getPreferredInternalFormat());
    tex->bind().setData(data->getPreferredInternalFormat(), data);
    return tex;
}

SharedTexture1D Texture1D::createFromData(const SharedTextureData &data, GLenum internalFormat)
{
    if (!data)
    {
        error() << "Texture1D::createFromData failed, no data provided";
        return nullptr;
    }

    auto tex = std::make_shared<Texture1D>(internalFormat);
    tex->bind().setData(internalFormat, data);
    return tex;
}

bool Texture1D::BoundTexture1D::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture " << to_string(texture), return false);
    return true;
}

void Texture1D::BoundTexture1D::makeStorageImmutable(int width, GLenum internalFormat, int mipmapLevels)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is already immutable " << to_string(texture), return );
    checkValidGLOW();

    if (mipmapLevels <= 0)
        mipmapLevels = tg::floor(tg::log2((float)width)) + 1;

    texture->mStorageImmutable = true;
    texture->mInternalFormat = internalFormat;
    texture->mImmutableMipmaps = mipmapLevels;
    texture->mWidth = width;

    glTexStorage1D(texture->mTarget, mipmapLevels, internalFormat, width);
}

void Texture1D::BoundTexture1D::setMinFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void Texture1D::BoundTexture1D::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void Texture1D::BoundTexture1D::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void Texture1D::BoundTexture1D::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = tg::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void Texture1D::BoundTexture1D::setBorderColor(tg::color4 const& color)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterfv(texture->mTarget, GL_TEXTURE_BORDER_COLOR, &color.r);
    texture->mBorderColor = color;
}

void Texture1D::BoundTexture1D::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}

void Texture1D::BoundTexture1D::setWrap(GLenum wrapS)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
}


void Texture1D::BoundTexture1D::generateMipmaps()
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glGenerateMipmap(texture->mTarget);
    texture->mMipmapsGenerated = true;
}

void Texture1D::BoundTexture1D::setLodBias(float bias)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_LOD_BIAS, bias);
    texture->mLodBias = bias;
}

void Texture1D::BoundTexture1D::setBaseLevel(int lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_BASE_LEVEL, lvl);
    texture->mBaseLevel = lvl;
}

void Texture1D::BoundTexture1D::setMaxLevel(int lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAX_LEVEL, lvl);
    texture->mMaxLevel = lvl;
}

void Texture1D::BoundTexture1D::setMinLod(float lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_MIN_LOD, lvl);
    texture->mMinLod = lvl;
}

void Texture1D::BoundTexture1D::setMaxLod(float lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_LOD, lvl);
    texture->mMaxLod = lvl;
}

void Texture1D::BoundTexture1D::resize(int width)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is storage immutable " << to_string(texture), return );

    texture->mWidth = width;

    GLenum format = GL_RGBA;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        break;
    }
    switch (texture->mInternalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        format = GL_DEPTH_COMPONENT;
        break;

    case GL_DEPTH32F_STENCIL8:
    case GL_DEPTH24_STENCIL8:
        format = GL_DEPTH_STENCIL;
        break;

    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1:
    case GL_STENCIL_INDEX4:
    case GL_STENCIL_INDEX8:
    case GL_STENCIL_INDEX16:
        format = GL_STENCIL_INDEX;
        break;
    }

    glTexImage1D(texture->mTarget, 0, texture->mInternalFormat, width, 0, format, GL_UNSIGNED_BYTE, nullptr);

    texture->mMipmapsGenerated = false;
}

void Texture1D::BoundTexture1D::setData(GLenum internalFormat, int width, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    if (texture->isStorageImmutable())
    {
        TG_ASSERT(mipmapLevel == 0 && "not implemented for higher levels");
        GLOW_RUNTIME_ASSERT(texture->mWidth == width, "Texture is storage immutable and a wrong width was specified " << to_string(texture), return );
        GLOW_RUNTIME_ASSERT(texture->mInternalFormat == internalFormat,
                            "Texture is storage immutable and a wrong internal format was specified " << to_string(texture), return );
    }

    if (mipmapLevel == 0)
    {
    texture->mWidth = width;
        texture->mMipmapsGenerated = false;
        texture->mInternalFormat = internalFormat;
    }

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    if (texture->isStorageImmutable())
        glTexSubImage1D(texture->mTarget, mipmapLevel, 0, width, format, type, data);
    else
        glTexImage1D(texture->mTarget, mipmapLevel, texture->mInternalFormat, width, 0, format, type, data);
}

void Texture1D::BoundTexture1D::setSubData(int x, int width, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    glTexSubImage1D(texture->mTarget, mipmapLevel, x, width, format, type, data);
 }

void Texture1D::BoundTexture1D::setData(GLenum internalFormat, const SharedTextureData &data)
{
    if (!isCurrent())
        return;

    texture->mInternalFormat = internalFormat; // format first, then resize
    resize(data->getWidth());

    // set all level 0 surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() == 0)
            setSubData(surf->getOffsetX(),
                       surf->getWidth(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());

    // set parameters
    if (data->getAnisotropicFiltering() >= 1.f)
        setAnisotropicFiltering(data->getAnisotropicFiltering());
    if (data->getMinFilter() != GL_INVALID_ENUM)
        setMinFilter(data->getMinFilter());
    if (data->getMagFilter() != GL_INVALID_ENUM)
        setMagFilter(data->getMagFilter());
    if (data->getBorderColor() != tg::color4(-1))
        setBorderColor(data->getBorderColor());
    if (data->getWrapS() != GL_INVALID_ENUM)
        setWrapS(data->getWrapS());

    // generate mipmaps
    if (texture->hasMipmapsEnabled())
        generateMipmaps();

    // set all level 1+ surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() > 0)
            setSubData(surf->getOffsetX(),
                       surf->getWidth(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());
}

void Texture1D::clear(GLenum format, GLenum type, const GLvoid* data, int mipmapLevel)
{
    checkValidGLOW();
#if GLOW_OPENGL_VERSION >= 44

    if (OGLVersion.total < 44)
    {
        glow::warning() << "Using fallback for Texture::clear because OpenGL Version is lower than 4.4.";
        glow::warning() << "  This has (severe) performance implications (see #43) " << to_string(this);

        glow::error() << "Not implemented. " << to_string(this);
    }
    else
    {
        glClearTexImage(mObjectName, mipmapLevel, format, type, data);
    }
#else
    error() << "Texture1D::clear is only supported for OpenGL 4.4+ " << to_string(this);
#endif
    mMipmapsGenerated = false;
}

std::vector<char> Texture1D::BoundTexture1D::getData(GLenum format, GLenum type, int mipmapLevel)
{
    if (!isCurrent())
        return {};
    checkValidGLOW();

    auto target = texture->mTarget;

    size_t dataSize = channelsOfFormat(format) * sizeOfTypeInBytes(type);

    GLint sWidth;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_WIDTH, &sWidth);
    dataSize *= sWidth;

    std::vector<char> data(dataSize);
    getData(format, type, data.size(), data.data(), mipmapLevel);
    return data;
}

void Texture1D::BoundTexture1D::getData(GLenum format, GLenum type, size_t bufferSize, void *buffer, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    (void)bufferSize; // TODO: check me!
    glGetTexImage(texture->mTarget, mipmapLevel, format, type, buffer);
}

SharedTextureData Texture1D::BoundTexture1D::getTextureData(int maxMipLevel)
{
    if (!isCurrent())
        return nullptr;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    auto tex = std::make_shared<TextureData>();

    // format
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE; // TODO: 16bit, 32bit types
    size_t bytesPerTexel = 4;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        bytesPerTexel = 1;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        bytesPerTexel = 2;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        bytesPerTexel = 3;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        bytesPerTexel = 4;
        break;
    }

    // tex parameters
    // TODO

    // surfaces
    auto target = texture->mTarget;

    for (auto lvl = 0; lvl <= maxMipLevel; ++lvl)
    {
        GLint w;
        GLint h;
        GLint d;
        GLenum internalFormat;
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_DEPTH, &d);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internalFormat);


        if (w * h * d == 0)
            break; // no mipmaps any more

        if (lvl == 0)
        {
            tex->setWidth(w);
            tex->setHeight(h);
            tex->setDepth(d);
            tex->setPreferredInternalFormat(internalFormat);
        }

        auto surface = std::make_shared<SurfaceData>();
        surface->setWidth(w);
        surface->setHeight(h);
        surface->setDepth(d);

        surface->setMipmapLevel(lvl);

        surface->setFormat(format);
        surface->setType(type);

        std::vector<char> data;
        data.resize(bytesPerTexel * w * h * d);
        glGetTexImage(target, lvl, surface->getFormat(), surface->getType(), data.data());
        surface->setData(data);

        tex->addSurface(surface);
    }

    return tex;
}

void Texture1D::BoundTexture1D::writeToFile(const std::string &filename)
{
    getTextureData()->saveToFile(filename);
}



Texture1D::BoundTexture1D::BoundTexture1D (Texture1D *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

Texture1D::BoundTexture1D::BoundTexture1D (Texture1D::BoundTexture1D &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
    sCurrentTexture = this;
}

Texture1D::BoundTexture1D::~BoundTexture1D ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
