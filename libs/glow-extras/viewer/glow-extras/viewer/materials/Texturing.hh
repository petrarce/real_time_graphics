#pragma once

#include <glow/fwd.hh>
#include <glow/objects/Program.hh>
#include <glow/util/AsyncTexture.hh>

#include "../detail/MeshAttribute.hh"
#include "../detail/MeshShaderBuilder.hh"
#include "../fwd.hh"

#include <typed-geometry/tg-lean.hh>

namespace glow
{
namespace viewer
{
class Texturing
{
    struct ExplicitTexture : AsyncTextureBase
    {
        SharedTexture mTexture;

        ExplicitTexture(SharedTexture tex) : mTexture(move(tex)) {}

        SharedTexture queryTexture() const override { return mTexture; }
    };

private:
    std::shared_ptr<AsyncTextureBase> mTexture;
    detail::SharedMeshAttribute mCoordsAttribute;
    tg::mat3 mTextureTransform = tg::mat3::identity;

    void buildShader(detail::MeshShaderBuilder& shader) const;
    void prepareShader(UsedProgram& shader) const;

public:
    Texturing& texture(SharedTexture t)
    {
        mTexture = std::make_shared<ExplicitTexture>(move(t));
        return *this;
    }
    template <class T>
    Texturing& texture(AsyncTexture<T> const& t)
    {
        mTexture = std::make_shared<AsyncTexture<T>>(t);
        return *this;
    }
    template <class T>
    Texturing& coords(T const& data)
    {
        mCoordsAttribute = detail::make_mesh_attribute("aTexCoord", data);
        return *this;
    }
    Texturing& transform(tg::mat3 const& m)
    {
        mTextureTransform = m;
        return *this;
    }

    friend class MeshRenderable;
    friend class PointRenderable;
    friend class LineRenderable;
};
}
}
