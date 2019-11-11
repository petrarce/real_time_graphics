#include "ColorMapping.hh"

using namespace glow;
using namespace glow::viewer;

void ColorMapping::buildShader(glow::viewer::detail::MeshShaderBuilder &shader) const
{
    // TODO: other types
    if (mMode > Map1D)
    {
        glow::error() << "higher mappings not yet implemented";
        return;
    }

    if (!mTexture)
        glow::error() << "ColorMapping has no texture";

    {
        auto tex1D = std::dynamic_pointer_cast<glow::Texture1D>(mTexture);
        if (tex1D)
            tex1D->bind().setWrap(mClamped ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        else
            glow::error() << "ColorMapping has wrong texture type";
    }

    shader.addPassthrough(mDataAttribute->typeInShader(), "Data");
    shader.addPassthrough("vec4", "Color");

    shader.addUniform("sampler1D", "uTexMapping");
    shader.addUniform("vec3", "uDataMin");
    shader.addUniform("vec3", "uDataMax");
    shader.addFragmentShaderCode("vColor = texture(uTexMapping, (vData - uDataMin.x) / (uDataMax.x - "
                                 "uDataMin.x));");
}

void ColorMapping::prepareShader(UsedProgram &shader) const
{
    shader.setTexture("uTexMapping", mTexture);
    shader.setUniform("uDataMin", mDataMin);
    shader.setUniform("uDataMax", mDataMax);
}
