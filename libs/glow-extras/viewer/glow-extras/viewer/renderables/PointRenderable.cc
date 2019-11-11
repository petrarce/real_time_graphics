#include "PointRenderable.hh"

#include "../RenderInfo.hh"
#include "../detail/MeshShaderBuilder.hh"

using namespace glow;
using namespace glow::viewer;

aabb PointRenderable::computeAabb()
{
    auto aabb = getMeshAABB().transformed(transform());
    if (mWorldSpaceSize)
    {
        const auto maxRadius = getAttribute("aPointSize")->computeMaxFloat() / 2.0f;
        aabb.min -= tg::vec3(maxRadius);
        aabb.max += tg::vec3(maxRadius);
    }
    return aabb;
}


void PointRenderable::renderPoints(const RenderInfo& info)
{
    auto shader = mForwardShader->use();

    shader["uModel"] = transform();
    shader["uView"] = info.view;
    shader["uProj"] = info.proj;
    shader["uCamPos"] = info.camPos;
    shader["uFresnel"] = getFresnel();
    shader["uIsTransparent"] = getRenderMode() == GeometricRenderable::RenderMode::Transparent;
    shader["uSeed"] = tg::u32(info.accumulationCount);
    shader.setUniform("uScreenSize", tg::vec2(info.resolution));
    shader.setUniform("uInvModel", inverse(transform()));
    shader.setUniform("uInvView", tg::inverse(info.view));
    shader.setUniform("uInvProj", tg::inverse(info.proj));

    if (mRenderMode == RenderMode::Sphere)
    {
        shader.setUniform("uViewTranspose", tg::transpose(info.view));
        auto const tanHalfFovY = 1.f / tg::abs(info.proj[1][1]);
        shader.setUniform("uTanFov2", tanHalfFovY);
        shader.setUniform("uScreenHeight", float(info.resolution.height));
    }

    if (getColorMapping())
        getColorMapping()->prepareShader(shader);
    if (getTexturing())
        getTexturing()->prepareShader(shader);
    for (auto const& a : getAttributes())
        a->prepareShader(shader);

    mVertexArray->bind().draw();
}

void PointRenderable::renderShadow(RenderInfo const& info)
{
    auto shader = mShadowShader->use();

    shader.setUniform("uModel", transform());
    shader.setUniform("uInvModel", inverse(transform()));
    shader.setUniform("uScreenSize", tg::vec2(info.resolution));
    shader.setUniform("uView", info.view);
    shader.setUniform("uInvView", tg::inverse(info.view));
    shader.setUniform("uProj", info.proj);
    shader.setUniform("uCamPos", info.camPos);

    if (mRenderMode == RenderMode::Sphere)
    {
        shader.setUniform("uViewTranspose", tg::transpose(info.view));
        auto const tanHalfFovY = 1.f / tg::abs(info.proj[1][1]);
        shader.setUniform("uTanFov2", tanHalfFovY);
        shader.setUniform("uScreenHeight", float(info.resolution.height));
    }

    for (auto const& a : getAttributes())
        a->prepareShader(shader);

    mVertexArray->bind().draw();
}

void PointRenderable::renderForward(RenderInfo const& info)
{
    if (getRenderMode() != GeometricRenderable::RenderMode::Opaque)
        return;

    renderPoints(info);
}

void PointRenderable::renderTransparent(const RenderInfo& info)
{
    if (getRenderMode() != GeometricRenderable::RenderMode::Transparent)
        return;

    renderPoints(info);
}

SharedPointRenderable PointRenderable::create(const builder::PointBuilder& builder)
{
    auto r = std::make_shared<PointRenderable>();
    r->initFromBuilder(builder);
    return r;
}

void PointRenderable::initFromBuilder(const builder::PointBuilder& builder)
{
    initGeometry(builder.getMeshDef(), builder.getAttributes());

    if (builder.mPointMode == builder::PointBuilder::PointMode::Round)
        mRenderMode = RenderMode::Round;
    else if (builder.mPointMode == builder::PointBuilder::PointMode::Square)
        mRenderMode = RenderMode::Square;
    else if (builder.mPointMode == builder::PointBuilder::PointMode::Spheres)
        mRenderMode = RenderMode::Sphere;
    else
        mRenderMode = RenderMode::Sphere; // default;

    if (builder.mWorldSpaceSize)
        mWorldSpaceSize = true;
    else if (builder.mScreenSpaceSize)
        mWorldSpaceSize = false;
    else
        mWorldSpaceSize = false; // default

    if (builder.mCameraFacing)
        mCameraFacing = true;
    else if (builder.mOwnNormals)
        mCameraFacing = false;
    else
        mCameraFacing = true; // default

    mUnlit = builder.mUnlit;
    if (mUnlit && mRenderMode == RenderMode::Sphere)
    {
        glow::warning() << "Sphere points are always lit";
        mUnlit = false;
    }
}

void PointRenderable::init()
{
    // add missing attributes
    if (!hasAttribute("aPointSize"))
        addAttribute(detail::make_mesh_attribute("aPointSize", 10.0f));
    if (!hasAttribute("aNormal"))
        addAttribute(detail::make_mesh_attribute("aNormal", glm::vec3(0, 1, 0)));

    if (getTexturing())
        addAttribute(getTexturing()->mCoordsAttribute);
    else if (getColorMapping())
        addAttribute(getColorMapping()->mDataAttribute);
    else if (!hasAttribute("aColor"))
    {
        auto const color = glm::vec4(glm::vec3(mCameraFacing && mRenderMode != RenderMode::Sphere ? 0.f : .7f), 1.0f);
        addAttribute(detail::make_mesh_attribute("aColor", color));
    }

    auto aColor = getAttribute("aColor");

    // build meshes
    {
        std::vector<SharedArrayBuffer> abs;

        for (auto const& attr : getAttributes())
        {
            auto ab = attr->createPointRenderableArrayBuffer(*getMeshDefinition());
            if (!ab)
                continue;

            ab->setDivisor(1);
            abs.push_back(ab);
        }

        {
            std::vector<glm::vec2> quadPos;

            quadPos.emplace_back(0, 0);
            quadPos.emplace_back(1, 1);
            quadPos.emplace_back(0, 1);

            quadPos.emplace_back(0, 0);
            quadPos.emplace_back(1, 0);
            quadPos.emplace_back(1, 1);

            auto qad = ArrayBuffer::create();
            qad->defineAttribute<glm::vec2>("aQuadPos");
            qad->bind().setData(quadPos);
            abs.push_back(qad);
        }

        mVertexArray = VertexArray::create(abs);
    }

    // build shader
    {
        // Shared functionality
        auto createNonFragmentShaderParts = [&](detail::MeshShaderBuilder& sb) {
            sb.addUniform("mat4", "uModel");
            sb.addUniform("mat4", "uInvModel");

            sb.addUniform("mat4", "uView");
            sb.addUniform("mat4", "uInvView");
            sb.addUniform("mat4", "uProj");
            sb.addUniform("mat4", "uInvProj");
            sb.addUniform("vec3", "uCamPos");


            sb.addUniform("vec2", "uScreenSize");
            if (mRenderMode == RenderMode::Sphere)
            {
                sb.addUniform("mat4", "uViewTranspose");
                sb.addUniform("float", "uTanFov2");
                sb.addUniform("float", "uScreenHeight");
            }

            sb.addAttribute("vec2", "aQuadPos");

            sb.addPassthrough("vec2", "QuadPos");
            sb.addPassthrough("vec3", "Normal");

            for (auto const& attr : getAttributes())
                attr->buildShader(sb);

            if (mRenderMode == RenderMode::Sphere)
            {
                sb.addPassthrough("vec3", "SphereCenter");
                sb.addPassthrough("float", "SphereRadius");
                sb.addPassthrough("vec3", "fragPosWS");

                if (mWorldSpaceSize)
                {
                    // world space size spheres
                    sb.addVertexShaderCode(R"(
                        vOut.SphereRadius = aPointSize;
                        vOut.SphereCenter = vec3(uModel * vec4(aPosition, 1.0));

                        vec4 vpos = uView * uModel * vec4(aPosition, 1.0);
                        vec3 fwd = normalize(vpos.xyz);
                        vec3 left = normalize(cross(vec3(0,1,0), fwd));
                        vec3 up = cross(fwd, left);
                        vec2 qp = aQuadPos * 2 - 1;
                        vpos.xyz -= left * qp.x * aPointSize;
                        vpos.xyz += up * qp.y * aPointSize;
                        vpos.xyz -= fwd * aPointSize;

                        vpos = uInvModel * uInvView * vpos;

                        vOut.Normal = vec3(0,0,0);
                        vOut.fragPosWS = vec3(uModel * vec4(vpos.xyz, 1.0));
                        gl_Position = uProj * uView * uModel * vec4(vpos.xyz, 1.0);
                    )");
                }
                else
                {
                    // screen space size spheres
                    sb.addVertexShaderCode(R"(
                        vec3 pos = vec3(uModel * vec4(aPosition, 1.0));
                        float l = 2 * distance(pos, uCamPos) * uTanFov2;
                        float s = l * aPointSize / uScreenHeight;
                        vOut.SphereRadius = s;
                        vOut.SphereCenter = pos;

                        vec4 vpos = uView * uModel * vec4(aPosition, 1.0);
                        vec3 fwd = normalize(vpos.xyz);
                        vec3 left = normalize(cross(vec3(0,1,0), fwd));
                        vec3 up = cross(fwd, left);
                        vec2 qp = aQuadPos * 2 - 1;
                        vpos.xyz -= left * qp.x * s;
                        vpos.xyz += up * qp.y * s;
                        vpos.xyz -= fwd * s;

                        vpos = uInvModel * uInvView * vpos;

                        vOut.Normal = vec3(0,0,0);
                        vOut.fragPosWS = vec3(uModel * vec4(vpos.xyz, 1.0));
                        gl_Position = uProj * uView * uModel * vec4(vpos.xyz, 1.0);
                    )");
                }
            }
            else if (mCameraFacing && !mWorldSpaceSize)
            {
                // camera facing screen space size
                sb.addVertexShaderCode(R"(
                    vec4 spos = uProj * uView * uModel * vec4(aPosition, 1.0);
                    spos /= spos.w;

                    vec2 qp = aQuadPos * 2 - 1;
                    spos.xy += qp * aPointSize / uScreenSize;

                    spos = uInvProj * spos;
                    spos /= spos.w;
                    spos = uInvModel * uInvView * spos;

                    vOut.Normal = vec3(0,0,0);
                    gl_Position = uProj * uView * uModel * vec4(spos.xyz, 1.0);
                )");
            }
            else if (mCameraFacing)
            {
                // camera facing world space size
                sb.addVertexShaderCode(R"(
                    vec4 vpos = uView * uModel * vec4(aPosition, 1.0);
                    vec2 qp = aQuadPos * 2 - 1;
                    vpos.xy += qp * aPointSize;

                    vpos = uInvModel * uInvView * vpos;

                    vOut.Normal = vec3(0,0,0);
                    gl_Position = uProj * uView * uModel * vec4(vpos.xyz, 1.0);
                )");
            }
            else
            {
                // Normal aligned world space size
                sb.addVertexShaderCode(R"(
                    vec3 pos = vec3(uModel * vec4(aPosition, 1.0));
                    vec3 dir = normalize(mat3(uModel) * aNormal);
                    vec3 up = abs(dir.y) < abs(dir.x) ? vec3(0, 1, 0) : vec3(1, 0, 0);
                    vec3 left = normalize(cross(dir, up));
                    up = normalize(cross(dir, left));
                    vec2 qp = aQuadPos - 0.5;
                    vec3 wpos = pos + aPointSize * (left * qp.x + up * qp.y);

                    vOut.Normal = dir;
                    gl_Position = uProj * uView * vec4(wpos, 1.0);
                )");
            }
        };


        // Forward
        {
            detail::MeshShaderBuilder sbForward;
            createNonFragmentShaderParts(sbForward);

            sbForward.addFragmentLocation("vec4", "fColor");
            sbForward.addFragmentLocation("vec3", "fNormal");

            sbForward.addUniform("bool", "uIsTransparent");
            sbForward.addUniform("bool", "uFresnel");
            sbForward.addUniform("uint", "uSeed");

            sbForward.addPassthrough("float", "VertexID");
            sbForward.addVertexShaderCode(R"(
                                    vOut.VertexID = float(gl_VertexID);
                                   )");

            // colored mesh
            if (aColor)
                sbForward.addPassthrough(aColor->typeInShader(), "Color");

            // data mapped mesh
            if (getColorMapping())
                getColorMapping()->buildShader(sbForward);

            // texture mesh
            if (getTexturing())
                getTexturing()->buildShader(sbForward);

            if (mRenderMode == RenderMode::Sphere)
            {
                sbForward.addFragmentShaderDecl(R"(
                     float distance2(vec3 a, vec3 b)
                     {
                         vec3 d = a - b;
                         return dot(d, d);
                     }
                )");

                sbForward.addFragmentShaderCode(R"(
                    vec3 rayOrigin = uCamPos;
                    vec3 rayDir = normalize(vIn.fragPosWS - uCamPos);

                    vec3 closestP = rayOrigin + rayDir * dot(rayDir, vSphereCenter - rayOrigin);
                    float sphereDis2 = distance2(closestP, vSphereCenter);

                    if (sphereDis2 > vSphereRadius * vSphereRadius)
                        discard;

                    vec3 spherePos = closestP - rayDir * sqrt(vSphereRadius * vSphereRadius - sphereDis2);

                    vec3 sphereN = normalize(spherePos - vSphereCenter);

                    vec4 spherePosCS = uProj * uView * vec4(spherePos, 1);
                    float depthNDC = spherePosCS.z / spherePosCS.w;

                    gl_FragDepth = (depthNDC - gl_DepthRange.near) / gl_DepthRange.diff;

                    fColor.rgb = vec3(vColor) * (sphereN.y * .5 + .5);
                    fColor.a = 1;
                    fNormal = sphereN;
                )");
            }
            else
            {
                // Quads and Circles
                if (mRenderMode == RenderMode::Round)
                    sbForward.addFragmentShaderCode("if (distance(vQuadPos, vec2(0.5)) > 0.5) discard;");

                if (mUnlit)
                    sbForward.addFragmentShaderCode(R"(
                                             fNormal = normalize(vNormal);
                                             fColor.rgb = vec3(vColor);
                                             fColor.a = 1;
                                             )");
                else
                    sbForward.addFragmentShaderCode(R"(
                                             fNormal = normalize(vNormal);
                                             fColor.rgb = vec3(vColor) * (fNormal.y * .5 + .5);
                                             fColor.a = 1;
                                             )");
            }

            if (getRenderMode() == GeometricRenderable::RenderMode::Transparent)
                sbForward.addFragmentShaderCode(R"(
                                         if (uIsTransparent)
                                         {
                                            uint h = uint(gl_FragCoord.x) * 4096 + uint(gl_FragCoord.y);
                                            h = hash_combine(h, uint(uSeed));
                                            h = hash_combine(h, uint(vVertexID * 17));
                                            h = wang_hash(h);

                                            float a = vColor.a;

                                            if (uFresnel)
                                            {
                                                vec3 V = normalize(uCamPos - vIn.fragPosWS);
                                                float t = 1 - abs(dot(fNormal, V));
                                                t = (t * t) * (t * t) * t;
                                                a = mix(vColor.a, 1, t);
                                            }

                                            if (wang_float(h) > a)
                                                discard;
                                         })");

            mForwardShader = sbForward.createProgram();
        }

        // Shadow
        {
            detail::MeshShaderBuilder sbShadow;
            createNonFragmentShaderParts(sbShadow);

            if (mRenderMode == RenderMode::Sphere)
            {
                sbShadow.addFragmentShaderDecl(R"(
                     float distance2(vec3 a, vec3 b)
                     {
                         vec3 d = a - b;
                         return dot(d, d);
                     }
                )");

                sbShadow.addFragmentShaderCode(R"(
                    vec3 rayOrigin = uCamPos;
                    vec3 rayDir = normalize(vIn.fragPosWS - uCamPos);

                    vec3 closestP = rayOrigin + rayDir * dot(rayDir, vSphereCenter - rayOrigin);
                    float sphereDis2 = distance2(closestP, vSphereCenter);
                    if (sphereDis2 > vSphereRadius * vSphereRadius)
                        discard;
                )");
            }
            else
            {
                // Quads and Circles
                if (mRenderMode == RenderMode::Round)
                    sbShadow.addFragmentShaderCode("if (distance(vQuadPos, vec2(0.5)) > 0.5) discard;");
            }

            mShadowShader = sbShadow.createProgram();
        }
    }
}
