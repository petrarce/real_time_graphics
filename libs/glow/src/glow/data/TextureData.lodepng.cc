#include "SurfaceData.hh"
#include "TextureData.hh"

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

#include <glow/detail/lodepng/lodepng.hh>

using namespace glow;

SharedTextureData TextureData::loadWithLodepng(const std::string& filename, std::string const&, ColorSpace colorSpace)
{
    GLOW_ACTION();

    std::vector<unsigned char> data; // the raw pixels
    unsigned width, height;

    // decode
    unsigned errCode = glow_lodepng::decode(data, width, height, filename);

    // if there's an error, display it
    if (errCode)
    {
        error() << "Error loading LodePNG texture data from " << filename;
        error() << "  decoder error " << errCode << ": " << glow_lodepng_error_text(errCode);
        return nullptr;
    }

    auto tex = std::make_shared<TextureData>();
    auto surface = std::make_shared<SurfaceData>();

    surface->setData(std::vector<char>(data.data(), data.data() + data.size()));

    surface->setType(GL_UNSIGNED_BYTE);
    surface->setMipmapLevel(0);
    surface->setWidth(static_cast<int>(width));
    surface->setHeight(static_cast<int>(height));
    surface->setFormat(GL_RGBA);

    switch (colorSpace)
    {
    case ColorSpace::Linear:
        tex->setPreferredInternalFormat(GL_RGBA);
        break;
    case ColorSpace::sRGB:
        tex->setPreferredInternalFormat(GL_SRGB_ALPHA);
        break;
    }
    tex->setWidth(static_cast<int>(width));
    tex->setHeight(static_cast<int>(height));
    tex->addSurface(surface);

    // high-quality default parameters:
    tex->setAnisotropicFiltering(16.0f);
    tex->setMagFilter(GL_LINEAR);
    tex->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);

    return tex;
}

SharedTextureData TextureData::loadWithLodepng(const unsigned char* rawData, size_t rawDataSize, ColorSpace colorSpace)
{
    GLOW_ACTION();

    std::vector<unsigned char> data; // the raw pixels
    unsigned width, height;

    // decode
    unsigned errCode = glow_lodepng::decode(data, width, height, rawData, rawDataSize);

    // if there's an error, display it
    if (errCode)
    {
        error() << "Error loading LodePNG texture data from raw data";
        error() << "  decoder error " << errCode << ": " << glow_lodepng_error_text(errCode);
        return nullptr;
    }

    auto tex = std::make_shared<TextureData>();
    auto surface = std::make_shared<SurfaceData>();

    surface->setData(std::vector<char>(data.data(), data.data() + data.size()));

    surface->setType(GL_UNSIGNED_BYTE);
    surface->setMipmapLevel(0);
    surface->setWidth(static_cast<int>(width));
    surface->setHeight(static_cast<int>(height));
    surface->setFormat(GL_RGBA);

    switch (colorSpace)
    {
    case ColorSpace::Linear:
        tex->setPreferredInternalFormat(GL_RGBA);
        break;
    case ColorSpace::sRGB:
        tex->setPreferredInternalFormat(GL_SRGB_ALPHA);
        break;
    }
    tex->setWidth(static_cast<int>(width));
    tex->setHeight(static_cast<int>(height));
    tex->addSurface(surface);

    // high-quality default parameters:
    tex->setAnisotropicFiltering(16.0f);
    tex->setMagFilter(GL_LINEAR);
    tex->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);

    return tex;
}

void TextureData::saveWithLodepng(TextureData* data, const std::string& filename, const std::string&)
{
    GLOW_ACTION();

    SharedSurfaceData tmpS;
    for (auto const& surf : data->getSurfaces())
    {
        if (surf->getMipmapLevel() == 0)
        {
            if (surf->getType() != GL_UNSIGNED_BYTE)
            {
                error() << "Error saving LodePNG texture data to " << filename;
                error() << "  Reason: type " << surf->getType() << " is not supported with lodepng (yet).";
                return;
            }

            switch (surf->getFormat())
            {
            case GL_RGBA:
                glow_lodepng_encode32_file(filename.c_str(),                             //
                                      (unsigned char const*)surf->getFlippedData().data(), //
                                      surf->getWidth(),                             //
                                      surf->getHeight());
                return;

            case GL_BGRA:
                tmpS = surf->convertTo(GL_RGBA, surf->getType());
                glow_lodepng_encode32_file(filename.c_str(),                             //
                                      (unsigned char const*)tmpS->getFlippedData().data(), //
                                      surf->getWidth(),                             //
                                      surf->getHeight());
                return;

            case GL_RGB:
                glow_lodepng_encode24_file(filename.c_str(),                                    //
                                      (unsigned char const*)surf->getFlippedData().data(), //
                                      surf->getWidth(),                                    //
                                      surf->getHeight());

                return;
            case GL_RED:
            case GL_RG:
            case GL_BGR:
                tmpS = surf->convertTo(GL_RGB, surf->getType());
                glow_lodepng_encode24_file(filename.c_str(),                             //
                                      (unsigned char const*)tmpS->getFlippedData().data(), //
                                      surf->getWidth(),                             //
                                      surf->getHeight());
                return;

            default:
                error() << "Error saving LodePNG texture data to " << filename;
                error() << "  Reason: format " << surf->getFormat() << " is not supported with lodepng.";
                return;
            }
        }
    }

    error() << "Error saving LodePNG texture data to " << filename;
    error() << "  Reason: No supported level 0 surface found.";
}
