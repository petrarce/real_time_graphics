#include "MeshRenderable.hh"

#include <glow/common/scoped_gl.hh>

#include "../RenderInfo.hh"
#include "../detail/MeshShaderBuilder.hh"

using namespace glow;
using namespace glow::viewer;

aabb MeshRenderable::computeAabb() { return getMeshAABB().transformed(transform()); }

void MeshRenderable::renderMesh(const RenderInfo& ctx)
{
    auto shader = mForwardShader->use();

    GLOW_SCOPED(onlyEnableIf, getBackfaceCullingEnabled(), GL_CULL_FACE);

    shader["uModel"] = transform();
    shader["uView"] = ctx.view;
    shader["uProj"] = ctx.proj;
    shader["uFresnel"] = getFresnel();
    shader["uIsTransparent"] = getRenderMode() == RenderMode::Transparent;
    shader["uSeed"] = tg::u32(ctx.accumulationCount);
    shader["uCamPos"] = ctx.camPos;

    if (getColorMapping())
        getColorMapping()->prepareShader(shader);
    if (getTexturing())
        getTexturing()->prepareShader(shader);
    for (auto const& a : getAttributes())
        a->prepareShader(shader);
    mMesh->bind().draw();
}

void MeshRenderable::renderShadow(RenderInfo const& ctx) { renderMesh(ctx); }

void MeshRenderable::renderForward(RenderInfo const& ctx)
{
    if (getRenderMode() != RenderMode::Opaque)
        return;

    renderMesh(ctx);
}

void MeshRenderable::renderTransparent(RenderInfo const& ctx)
{
    if (getRenderMode() != RenderMode::Transparent)
        return;

    renderMesh(ctx);
}

SharedMeshRenderable MeshRenderable::create(builder::PolygonBuilder const& builder)
{
    auto r = std::make_shared<MeshRenderable>();
    r->initGeometry(builder.getMeshDef(), builder.getAttributes());
    return r;
}

void MeshRenderable::init()
{
    // add missing attributes
    if (!hasAttribute("aNormal"))
        addAttribute(getMeshDefinition()->computeFaceNormalAttribute());
    if (getTexturing())
        addAttribute(getTexturing()->mCoordsAttribute);
    else if (getColorMapping())
        addAttribute(getColorMapping()->mDataAttribute);
    else if (!hasAttribute("aColor"))
        addAttribute(detail::make_mesh_attribute("aColor", glm::vec3(1, 1, 1)));

    auto aColor = getAttribute("aColor");

    // build meshes
    {
        std::vector<SharedArrayBuffer> abs;
        SharedArrayBuffer posAB;
        SharedElementArrayBuffer eab;

        for (auto const& attr : getAttributes())
        {
            auto ab = attr->createMeshRenderableArrayBuffer(*getMeshDefinition());
            if (!ab)
                continue;

            abs.push_back(ab);
        }

        mMesh = VertexArray::create(abs, getIndexBuffer());
    }

    // build shader
    {
        detail::MeshShaderBuilder sb;

        sb.addUniform("mat4", "uModel");
        sb.addUniform("mat4", "uProj");
        sb.addUniform("mat4", "uView");
        sb.addUniform("vec3", "uCamPos");
        sb.addUniform("bool", "uIsTransparent");
        sb.addUniform("bool", "uFresnel");
        sb.addUniform("uint", "uSeed");

        sb.addFragmentLocation("vec4", "fColor");
        sb.addFragmentLocation("vec3", "fNormal");

        sb.addPassthrough("vec3", "Normal");
        sb.addPassthrough("vec3", "WorldPos");
        sb.addPassthrough("float", "VertexID");

        for (auto const& attr : getAttributes())
            attr->buildShader(sb);

        // colored mesh
        if (aColor)
            sb.addPassthrough(aColor->typeInShader(), "Color");

        // data mapped mesh
        if (getColorMapping())
            getColorMapping()->buildShader(sb);

        // texture mesh
        if (getTexturing())
            getTexturing()->buildShader(sb);

        sb.addVertexShaderCode(R"(
                                gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
                                vOut.VertexID = float(gl_VertexID);
                                vOut.WorldPos = vec3(uModel * vec4(aPosition, 1.0));
                               )");

        sb.addFragmentShaderCode(R"(
                                 fNormal = normalize(vNormal) * (gl_FrontFacing ? 1 : -1);
                                 fColor.rgb = vColor.rgb * (fNormal.y * .4 + .6);
                                 fColor.a = 1;
                                 )");

        if (getRenderMode() == RenderMode::Transparent)
            sb.addFragmentShaderCode(R"(
                                     if (uIsTransparent)
                                     {
                                        uint h = uint(gl_FragCoord.x) * 4096 + uint(gl_FragCoord.y);
                                        h = hash_combine(h, uint(uSeed));
                                        h = hash_combine(h, uint(vVertexID * 17));
                                        h = wang_hash(h);

                                        float a = vColor.a;

                                        if (uFresnel)
                                        {
                                            vec3 V = normalize(uCamPos - vWorldPos);
                                            float t = 1 - abs(dot(fNormal, V));
                                            t = (t * t) * (t * t) * t;
                                            a = mix(vColor.a, 1, t);
                                        }

                                        if (wang_float(h) > a)
                                            discard;
                                     })");

        mForwardShader = sb.createProgram();
    }
}
