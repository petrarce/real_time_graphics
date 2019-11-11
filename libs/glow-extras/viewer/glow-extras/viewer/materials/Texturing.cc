#include "Texturing.hh"

using namespace glow;
using namespace glow::viewer;

void Texturing::buildShader(glow::viewer::detail::MeshShaderBuilder &shader) const
{
    shader.addPassthrough(mCoordsAttribute->typeInShader(), "TexCoord");
    shader.addPassthrough("vec4", "Color");

    // TODO: other sampler types
    shader.addUniform("sampler2D", "uTexture");
    shader.addUniform("mat3", "uTextureTransform");

    shader.addFragmentShaderCode("vec3 uv = uTextureTransform * vec3(vTexCoord, 1);");
    shader.addFragmentShaderCode("vColor = texture(uTexture, uv.xy / uv.z);");
}

void Texturing::prepareShader(glow::UsedProgram &shader) const
{
    assert(mTexture && "no texture set");
    shader.setTexture("uTexture", mTexture->queryTexture());
    shader["uTextureTransform"] = mTextureTransform;
}
