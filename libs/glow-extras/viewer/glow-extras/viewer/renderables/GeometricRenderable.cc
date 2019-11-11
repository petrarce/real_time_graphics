#include "GeometricRenderable.hh"

using namespace glow;
using namespace glow::viewer;

void GeometricRenderable::addAttribute(viewer::detail::SharedMeshAttribute attr)
{
    if (hasAttribute(attr->name()))
    {
        glow::error() << "Attribute " << attr->name() << " already added";
        return;
    }
    mAttributes.emplace_back(move(attr));
}

bool GeometricRenderable::hasAttribute(const std::string& name) const
{
    for (auto const& attr : mAttributes)
        if (attr->name() == name)
            return true;

    return false;
}

viewer::detail::SharedMeshAttribute GeometricRenderable::getAttribute(const std::string& name) const
{
    for (auto const& attr : mAttributes)
        if (attr->name() == name)
            return attr;

    return nullptr;
}

void GeometricRenderable::initGeometry(viewer::detail::SharedMeshDefinition def, std::vector<viewer::detail::SharedMeshAttribute> attributes)
{
    TG_ASSERT(def != nullptr);

    mAttributes = move(attributes);

    addAttribute(def->computePositionAttribute());

    mIndexBuffer = def->computeIndexBuffer();
    mMeshAABB = def->computeAABB();
    mMeshDefinition = move(def);
}
