#pragma once

#include "Renderable.hh"

#include "../materials/ColorMapping.hh"
#include "../materials/Texturing.hh"

namespace glow
{
namespace viewer
{
class GeometricRenderable : public Renderable
{
public:
    enum class RenderMode
    {
        Opaque,
        Transparent
    };

private:
    aabb mMeshAABB;

    std::vector<detail::SharedMeshAttribute> mAttributes;
    std::shared_ptr<ColorMapping> mColorMapping;
    std::shared_ptr<Texturing> mTexturing;
    detail::SharedMeshDefinition mMeshDefinition;

    glow::SharedElementArrayBuffer mIndexBuffer;

    RenderMode mRenderMode = RenderMode::Opaque;
    bool mFresnel = true;
    bool mBackfaceCullingEnabled = false;

public:
    GLOW_PROPERTY(RenderMode);
    GLOW_PROPERTY(Fresnel);
    GLOW_PROPERTY(BackfaceCullingEnabled);
    GLOW_GETTER(MeshAABB);
    GLOW_GETTER(MeshDefinition);
    GLOW_GETTER(Attributes);
    GLOW_GETTER(ColorMapping);
    GLOW_GETTER(Texturing);
    GLOW_GETTER(IndexBuffer);

    void addAttribute(detail::SharedMeshAttribute attr);
    bool hasAttribute(std::string const& name) const;
    detail::SharedMeshAttribute getAttribute(std::string const& name) const;

    void setColorMapping(ColorMapping const& cm) { mColorMapping = std::make_shared<ColorMapping>(cm); }
    void setTexturing(Texturing const& t) { mTexturing = std::make_shared<Texturing>(t); }

protected:
    void initGeometry(detail::SharedMeshDefinition def, std::vector<detail::SharedMeshAttribute> attributes);
};
}
}
