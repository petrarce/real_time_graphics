#include "LineRenderable.hh"

#include <glow-extras/material/IBL.hh>

#include "../RenderInfo.hh"
#include "../detail/MeshShaderBuilder.hh"

using namespace glow;
using namespace glow::viewer;

aabb LineRenderable::computeAabb()
{
    auto aabb = getMeshAABB().transformed(transform());
    if (mWorldSpaceSize)
    {
        const auto maxRadius = getAttribute("aLineWidth")->computeMaxFloat() / 2.0f;
        aabb.min -= tg::vec3(maxRadius);
        aabb.max += tg::vec3(maxRadius);
    }
    return aabb;
}

void LineRenderable::renderShadow(RenderInfo const& info)
{
    auto shader = mShadowShader->use();

    shader.setUniform("uModel", transform());
    shader.setUniform("uInvModel", inverse(transform()));
    shader.setUniform("uNoCaps", mNoCaps);
    shader.setUniform("uWorldSpaceSize", mWorldSpaceSize);
    shader.setUniform("uCameraFacing", mCameraFacing);
    shader.setUniform("uScreenSize", tg::vec2(info.resolution));
    shader.setUniform("uView", info.view);
    shader.setUniform("uInvView", tg::inverse(info.view));
    shader.setUniform("uInvProj", tg::inverse(info.proj));
    shader.setUniform("uProj", info.proj);
    shader.setUniform("uCamPos", info.camPos);
    if (m3D)
    {
        auto const tanHalfFovY = 1.f / tg::abs(info.proj[1][1]);
        shader.setUniform("uTanFov2", tanHalfFovY);
    }

    for (auto const& a : getAttributes())
        a->prepareShader(shader);

    mVertexArray->bind().draw();
}

void LineRenderable::renderForward(RenderInfo const& info)
{
    auto shader = mForwardShader->use();

    shader.setUniform("uModel", transform());
    shader.setUniform("uInvModel", inverse(transform()));
    shader.setUniform("uNoCaps", mNoCaps);
    shader.setUniform("uExtrapolate", mExtrapolate);
    shader.setUniform("uWorldSpaceSize", mWorldSpaceSize);
    shader.setUniform("uCameraFacing", mCameraFacing);
    shader.setUniform("uScreenSize", tg::vec2(info.resolution));
    shader.setUniform("uView", info.view);
    shader.setUniform("uInvView", tg::inverse(info.view));
    shader.setUniform("uInvProj", tg::inverse(info.proj));
    shader.setUniform("uProj", info.proj);
    shader.setUniform("uCamPos", info.camPos);
    if (m3D)
    {
        auto const tanHalfFovY = 1.f / tg::abs(info.proj[1][1]);
        shader.setUniform("uTanFov2", tanHalfFovY);
    }

    if (getColorMapping())
        getColorMapping()->prepareShader(shader);
    if (getTexturing())
        getTexturing()->prepareShader(shader);
    for (auto const& a : getAttributes())
        a->prepareShader(shader);

    mVertexArray->bind().draw();
}

SharedLineRenderable LineRenderable::create(const builder::LineBuilder& builder)
{
    auto r = std::make_shared<LineRenderable>();
    r->initFromBuilder(builder);
    return r;
}

void LineRenderable::initFromBuilder(const builder::LineBuilder& builder)
{
    initGeometry(builder.getMeshDef(), builder.getAttributes());

    if (builder.mRoundCaps)
        mRoundCaps = true;
    else if (builder.mSquareCaps)
        mRoundCaps = false;
    else if (builder.mNoCaps)
        mRoundCaps = false;
    else
        mRoundCaps = true; // default

    if (builder.mNoCaps)
        mNoCaps = true;
    else
        mNoCaps = false; // default

    if (builder.mExtrapolate)
        mExtrapolate = true;
    else if (builder.mNoExtrapolation)
        mExtrapolate = false;
    else
        mExtrapolate = false; // default

    if (builder.mForce3D)
        m3D = true;
    else if (builder.mCameraFacing)
        m3D = false;
    else if (builder.mOwnNormals)
        m3D = false;
    else
        m3D = true; // default

    if (builder.mForce3D)
        mCameraFacing = false;
    else if (builder.mCameraFacing)
        mCameraFacing = true;
    else if (builder.mOwnNormals)
        mCameraFacing = false;
    else
        mCameraFacing = false; // default, because m3D is true in this case

    if (builder.mWorldSpaceSize)
        mWorldSpaceSize = true;
    else if (builder.mScreenSpaceSize)
        mWorldSpaceSize = false;
    else
        mWorldSpaceSize = false; // default

    if (builder.mOwnNormals && !m3D && !builder.mWorldSpaceSize && !builder.mScreenSpaceSize)
        glow::warning() << "Normal aligned lines need some size information, since the default screen space size does not work in this case.";
}

void LineRenderable::init()
{
    // add missing attributes
    if (getTexturing())
        addAttribute(getTexturing()->mCoordsAttribute);
    else if (getColorMapping())
        addAttribute(getColorMapping()->mDataAttribute);
    else if (!hasAttribute("aColor"))
        addAttribute(detail::make_mesh_attribute("aColor", glm::vec4(glm::vec3(mCameraFacing ? 0.0f : 0.25f), 1.0f)));

    const auto aColor = getAttribute("aColor");
    const auto twoColored = aColor && aColor->hasTwoColoredLines();

    if (!hasAttribute("aLineWidth"))
        addAttribute(detail::make_mesh_attribute("aLineWidth", 5.0f));
    if (!hasAttribute("aNormal"))
    {
        addAttribute(detail::make_mesh_attribute("aNormal", glm::vec3(0, 1, 0)));
        
        if (twoColored && m3D)
            glow::warning() << "Two colored 3D lines need normal information. Please add them using normals(...) and then force3D() render mode.";
    }

    // build meshes
    {
        std::vector<SharedArrayBuffer> abs;

        for (auto const& attr : getAttributes())
        {
            auto ab = attr->createLineRenderableArrayBuffer(*getMeshDefinition());
            if (!ab)
                continue;

            abs.push_back(ab);
        }

        mVertexArray = VertexArray::create(abs, nullptr, GL_LINES);
    }

    // build shader
    {
        // Shared functionality
        const auto createCommonShaderParts = [&](detail::MeshShaderBuilder& sb) {
            sb.addUniform("mat4", "uModel");
            sb.addUniform("mat4", "uInvModel");
            sb.addUniform("bool", "uWorldSpaceSize");
            sb.addUniform("bool", "uCameraFacing");
            sb.addUniform("bool", "uNoCaps");
            sb.addUniform("vec2", "uScreenSize");
            if (m3D)
                sb.addUniform("float", "uTanFov2");

            sb.addUniform("mat4", "uView");
            sb.addUniform("mat4", "uInvView");
            sb.addUniform("mat4", "uProj");
            sb.addUniform("mat4", "uInvProj");
            sb.addUniform("vec3", "uCamPos");

            sb.addPassthrough("vec3", "Position");
            sb.addPassthrough("vec3", "Normal");
            sb.addPassthrough("float", "LineWidth");
            sb.addPassthrough("vec3", "fragPosWS");

            for (auto const& attr : getAttributes())
                attr->buildShader(sb);

            // Geometry shader in- and output
            sb.addGeometryShaderDecl("layout(lines) in;");
            if (m3D)
                sb.addGeometryShaderDecl("layout(triangle_strip, max_vertices = 6) out;"); // 6 vertices for 4 triangles
            else
                sb.addGeometryShaderDecl("layout(triangle_strip, max_vertices = 8) out;"); // 8 vertices for 6 triangles

            sb.addGeometryShaderDecl("out vec2 gAlpha;");
            sb.addFragmentShaderDecl("in vec2 gAlpha;");
            if (m3D)
            {
                sb.addGeometryShaderDecl("out vec3 gLineOrigin;");
                sb.addGeometryShaderDecl("out vec3 gLineEnd;");
                sb.addGeometryShaderDecl("out vec3 gLineDir;");
                sb.addFragmentShaderDecl("in vec3 gLineOrigin;");
                sb.addFragmentShaderDecl("in vec3 gLineEnd;");
                sb.addFragmentShaderDecl("in vec3 gLineDir;");
            }


            // Geometry shader code

            if (m3D)
            { // 3D lines
                sb.addGeometryShaderCode(R"(
    passthrough(0); // Load passthrough data for first vertex

    vec3 pos0 = vec3(uModel * vec4(vIn[0].Position, 1.0));
    vec3 pos1 = vec3(uModel * vec4(vIn[1].Position, 1.0));

    float s;
    if(uWorldSpaceSize) { // 3D world space lines
        s = vIn[0].LineWidth * 0.5;
    } else { // 3D screen space lines
        float l = 2 * distance(pos0, uCamPos) * uTanFov2;
        s = l * vIn[0].LineWidth / uScreenSize.y;
        vOut.LineWidth = 2 * s;
    }

    vec3 viewDir = normalize(pos0 - uCamPos);
    vec3 diff = pos1 - pos0;
    vec3 right = normalize(diff);
    vec3 up = normalize(cross(right, viewDir));
    vec3 back = normalize(cross(right, up));
    vec3 r = s * right;
    vec3 u = s * up;
    vec3 b = s * back;
    vec3 n = normalize(mat3(uModel) * vIn[0].Normal); // Needed for two colored 3D mode, but eventually always recomputed in the fragment shader
    float ea = s / length(diff); // Extrapolation alpha
    gLineOrigin = pos0; // Used in fragment shader
    gLineEnd = pos1;
    gLineDir = right;

    // Determine which line end is visible
    bool firstVisible = dot(viewDir, diff) > 0;

    // Quad at one line end
    float cr = uNoCaps ? 0 : -1;
    if(firstVisible) {
        createVertex(pos0 - b, ea, n, r, u, cr, 1);
        createVertex(pos0 - b, ea, n, r, u, cr, -1);
    }
    createVertex(pos0 + b, ea, n, r, u, cr, 1);
    createVertex(pos0 + b, ea, n, r, u, cr, -1);

    // Quad at other line end, reusing last two vertices to also create quad along line

    passthrough(1); // Load passthrough data for second vertex

    if(uWorldSpaceSize) {
        s = vIn[1].LineWidth * 0.5;
    } else {
        float l = 2 * distance(pos1, uCamPos) * uTanFov2;
        s = l * vIn[1].LineWidth / uScreenSize.y;
        vOut.LineWidth = 2 * s;
    }

    viewDir = normalize(pos1 - uCamPos);
    up = normalize(cross(right, viewDir));
    back = normalize(cross(right, up));
    r = s * right;
    u = s * up;
    b = s * back;
    n = normalize(mat3(uModel) * vIn[1].Normal);
    ea = s / length(diff);

    cr = uNoCaps ? 1 : 2;
    createVertex(pos1 + b, ea, n, r, u, cr, 1);
    createVertex(pos1 + b, ea, n, r, u, cr, -1);
    if(!firstVisible) {
        createVertex(pos1 - b, ea, n, r, u, cr, 1);
        createVertex(pos1 - b, ea, n, r, u, cr, -1);
    }
)");
            }
            else
            {
                if (mWorldSpaceSize)
                { // Flat world space lines
                    sb.addGeometryShaderCode(R"(
    passthrough(0); // Load passthrough data for first vertex

    vec3 pos0 = vec3(uModel * vec4(vIn[0].Position, 1.0));
    vec3 pos1 = vec3(uModel * vec4(vIn[1].Position, 1.0));
    vec3 normal0 = uCameraFacing ? normalize(uCamPos - pos0) : normalize(mat3(uModel) * vIn[0].Normal); // TODO: Correct?
    vec3 normal1 = uCameraFacing ? normalize(uCamPos - pos1) : normalize(mat3(uModel) * vIn[1].Normal);

    vec3 diff = pos1 - pos0;
    vec3 right = normalize(diff);
    vec3 up = normalize(cross(normal0, right));
    vec3 r = vIn[0].LineWidth * 0.5 * right;
    vec3 u = vIn[0].LineWidth * 0.5 * up;
    vec3 n = uCameraFacing ? vec3(0,0,0) : normal0; // If cameraFacing: Normal 0 signals unlit rendering
    float ea = vIn[0].LineWidth * 0.5 / length(diff); // Extrapolation alpha

    if(!uNoCaps) {
        createVertex(pos0, ea, n, r, u, -1, 1);
        createVertex(pos0, ea, n, r, u, -1, -1);
    }
    passthrough(0);
    createVertex(pos0, ea, n, r, u, 0, 1);
    createVertex(pos0, ea, n, r, u, 0, -1);

    up = normalize(cross(normal1, right));
    r = vIn[1].LineWidth * 0.5 * right;
    u = vIn[1].LineWidth * 0.5 * up;

    passthrough(1); // Load passthrough data for second vertex
    n = uCameraFacing ? vec3(0,0,0) : normal1;
    ea = vIn[1].LineWidth * 0.5 / length(diff);

    createVertex(pos1, ea, n, r, u, 1, 1);
    createVertex(pos1, ea, n, r, u, 1, -1);
    if(!uNoCaps) {
        createVertex(pos1, ea, n, r, u, 2, 1);
        createVertex(pos1, ea, n, r, u, 2, -1);
    }
)");
                }
                else
                { // Flat screen space lines
                    sb.addGeometryShaderCode(R"(
    passthrough(0); // Load passthrough data for first vertex

    // it only makes sense if uCameraFacing is true for screenSpaceSize lines
    vec3 n = vec3(0,0,0);
    vec4 spos0 = uProj * uView * uModel * vec4(vIn[0].Position, 1.0);
    vec4 spos1 = uProj * uView * uModel * vec4(vIn[1].Position, 1.0);

    if(spos0.w < 0 || spos1.w < 0) return; // Fix for lines nearer than the near plane

    spos0 /= spos0.w;
    spos1 /= spos1.w;

    vec2 diff = spos1.xy - spos0.xy;
    vec2 right = normalize(diff);
    vec2 up = vec2(-right.y, right.x);
    vec4 r = vec4(vIn[0].LineWidth * 0.5 * right / uScreenSize, 0, 0);
    vec4 u = vec4(vIn[0].LineWidth * 0.5 * up / uScreenSize, 0, 0);
    float ea = length(vIn[0].LineWidth * 0.5 / uScreenSize) / length(diff);

    if(!uNoCaps) {
        createVertexInverse(spos0, ea, n, r, u, -1, 1);
        createVertexInverse(spos0, ea, n, r, u, -1, -1);
    }
    passthrough(0);
    createVertexInverse(spos0, ea, n, r, u, 0, 1);
    createVertexInverse(spos0, ea, n, r, u, 0, -1);

    passthrough(1); // Load passthrough data for second vertex
    r = vec4(vIn[1].LineWidth * 0.5 * right / uScreenSize, 0, 0);
    u = vec4(vIn[1].LineWidth * 0.5 * up / uScreenSize, 0, 0);
    ea = length(vIn[1].LineWidth * 0.5 / uScreenSize) / length(diff);

    createVertexInverse(spos1, ea, n, r, u, 1, 1);
    createVertexInverse(spos1, ea, n, r, u, 1, -1);
    if(!uNoCaps) {
        createVertexInverse(spos1, ea, n, r, u, 2, 1);
        createVertexInverse(spos1, ea, n, r, u, 2, -1);
    }
)");
                }
            }


            // Fragment shader code

            if (m3D)
            {
                sb.addFragmentShaderDecl(R"(
                     float distance2(vec3 a, vec3 b) {
                         vec3 d = a - b;
                         return dot(d, d);
                     }
                )");

                // Camera ray: rayOrigin + tRay * rayDir;
                // Line: gLineOrigin + tLine * gLIneDir;
                // A: the angle between the Line and the Camera ray
                // r: the radius of the 2D slice of the capsule, i.e. its line width and radius of the two cap circles
                // s: the amount to go back on the ray dir to get to the intersection point
                // TODO in this shader code
                sb.addFragmentShaderCode(R"(
    vec3 rayOrigin = uCamPos;
    vec3 rayDir = normalize(vIn.fragPosWS - uCamPos);

    float cosA = dot(gLineDir, rayDir);
    float sinA2 = 1 - cosA * cosA;

    // TODO: Handle view parallel to line. Can for example be detected by sinA2 == 0

    // Compute closest points of the two lines
    vec3 origDiff = rayOrigin - gLineOrigin;
    float fRay = dot(rayDir, origDiff);
    float fLine = dot(gLineDir, origDiff);
    float tRay = (cosA * fLine - fRay) / sinA2;
    float tLine = (fLine - cosA * fRay) / sinA2;

    vec3 closestOnRay = rayOrigin + tRay * rayDir;
    vec3 closestOnLine = gLineOrigin + tLine * gLineDir;
    float lineRayDist2 = distance2(closestOnRay, closestOnLine);
    float lineRadius2 = vLineWidth * vLineWidth / 4; // vLineWidth is diameter, thus halved for radius

    if(lineRayDist2 > lineRadius2) discard;

    // Radius in 2D slice
    float r = sqrt(lineRadius2 - lineRayDist2);

    // Infinite cylinder intersection
    float s = r / sqrt(sinA2);
    vec3 cylIntersection = closestOnRay - s * rayDir;
    float tRayCyl = tRay - s;

    // Project onto line segment
    float lineLength = length(gLineEnd - gLineOrigin);
    float lambda = dot(cylIntersection - gLineOrigin, gLineDir); // TODO: QUESTION: Is there a better way?
    vec3 closestOnSegment = gLineOrigin + clamp(lambda, 0, lineLength) * gLineDir;

    // Ray-Sphere intersection same as in PointRenderable
    vec3 sphereCenter = closestOnSegment;
    float tRaySphere = dot(rayDir, sphereCenter - rayOrigin);
    vec3 closestP = rayOrigin + tRaySphere * rayDir;
    float sphereDis2 = distance2(closestP, sphereCenter);

    if(sphereDis2 > lineRadius2) discard;

    tRaySphere -= sqrt(lineRadius2 - sphereDis2); // Go back on ray to intersection

    vec3 newPos = rayOrigin + max(tRayCyl, tRaySphere) * rayDir;

    vec4 newPosCS = uProj * uView * vec4(newPos, 1);
    float depthNDC = newPosCS.z / newPosCS.w;
    gl_FragDepth = (depthNDC - gl_DepthRange.near) / gl_DepthRange.diff;
)");
            }
            else // Not 3D
            {
                if (mRoundCaps)
                {
                    sb.addFragmentShaderCode(R"(
    if(gAlpha.x < 0 && distance(gAlpha, vec2(0, 0)) > 1) discard;
    if(gAlpha.x > 1 && distance(gAlpha, vec2(1, 0)) > 1) discard;)");
                }
            }
        };
        // End of common


        // Forward
        {
            detail::MeshShaderBuilder sbForward;

            if (twoColored)
                sbForward.addGeometryShaderCode(R"(
    gRightColor = vIn[0].Color;
    gLeftColor = vIn[1].Color;
)");

            createCommonShaderParts(sbForward);

            sbForward.addUniform("bool", "uExtrapolate");

            sbForward.addFragmentLocation("vec4", "fColor");
            sbForward.addFragmentLocation("vec3", "fNormal");

            // colored mesh
            if (aColor)
                sbForward.addPassthrough(aColor->typeInShader(), "Color");

            if (twoColored)
            {
                sbForward.addGeometryShaderDecl("flat out vec4 gLeftColor;");
                sbForward.addGeometryShaderDecl("flat out vec4 gRightColor;");
                sbForward.addFragmentShaderDecl("flat in vec4 gLeftColor;");
                sbForward.addFragmentShaderDecl("flat in vec4 gRightColor;");
            }

            // data mapped mesh
            if (getColorMapping())
                getColorMapping()->buildShader(sbForward);

            // texture mesh
            if (getTexturing())
                getTexturing()->buildShader(sbForward);

            // Helper functions to emit vertices from the given relative line coordinates
            sbForward.addGeometryShaderDecl(R"(
void createVertex(vec3 basePos, float ea, vec3 n, vec3 r, vec3 u, float relX, float relY) {
    gAlpha = vec2(relX, relY); // Relative line position for fragment shader
    if(uExtrapolate) {
        if(relX < 0) passthroughMix(0, 1, relX * ea);
        if(relX > 1) passthroughMix(0, 1, (relX - 1) * ea + 1);
    }
    if(relX > 0) relX -= 1; // This sets the factor for the r vector to the right value in case of the second half

    vec3 outPos = basePos + relX * r + relY * u;
    gl_Position = uProj * uView * vec4(outPos, 1.0);

    vOut.fragPosWS =  outPos;
    vOut.Normal = n; // Set normal again as it might have been overwritten by passthrough
    EmitVertex();
}
void createVertexInverse(vec4 basePos, float ea, vec3 n, vec4 r, vec4 u, float relX, float relY) {
    gAlpha = vec2(relX, relY);
    if(uExtrapolate) {
        if(relX < 0) passthroughMix(0, 1, relX * ea);
        if(relX > 1) passthroughMix(0, 1, (relX - 1) * ea + 1);
    }
    if(relX > 0) relX -= 1;

    vec4 outPos = basePos + relX * r + relY * u;
    gl_Position = outPos;

    outPos = uInvProj * outPos;
    outPos /= outPos.w;
    outPos = uInvView * outPos;

    vOut.fragPosWS = vec3(outPos);
    vOut.Normal = n;
    EmitVertex();
}
)");

            // Fragment shader code

            if (twoColored && m3D)
                sbForward.addFragmentShaderCode(R"(
    vec3 leftDir = normalize(cross(vNormal, gLineDir));

    vNormal = normalize(newPos - closestOnSegment);

    vColor = dot(leftDir, vNormal) > 0 ? gLeftColor : gRightColor;)");

            else if (m3D)
                sbForward.addFragmentShaderCode("    vNormal = normalize(newPos - closestOnSegment);");

            else if (twoColored)
                sbForward.addFragmentShaderCode("    vColor = gAlpha.y > 0 ? gLeftColor : gRightColor;");

            // Rest is same for all versions
            sbForward.addFragmentShaderCode(R"(
    fNormal = vNormal;
    fColor.rgb = vColor.rgb * (fNormal.y * .5 + .5);
    fColor.a = 1;)");

            mForwardShader = sbForward.createProgram();
        }

        // Shadow
        {
            detail::MeshShaderBuilder sbShadow;
            createCommonShaderParts(sbShadow);

            // Helper functions to emit vertices from the given relative line coordinates
            sbShadow.addGeometryShaderDecl(R"(
void createVertex(vec3 basePos, float ea, vec3 n, vec3 r, vec3 u, float relX, float relY) {
    gAlpha = vec2(relX, relY); // Relative line position for fragment shader
    if(relX > 0) relX -= 1; // This sets the factor for the r vector to the right value in case of the second half

    vec3 outPos = basePos + relX * r + relY * u;
    gl_Position = uProj * uView * vec4(outPos, 1.0);

    vOut.fragPosWS =  outPos;
    EmitVertex();
}
void createVertexInverse(vec4 basePos, float ea, vec3 n, vec4 r, vec4 u, float relX, float relY) {
    gAlpha = vec2(relX, relY);
    if(relX > 0) relX -= 1;

    vec4 outPos = basePos + relX * r + relY * u;
    gl_Position = outPos;

    outPos = uInvProj * outPos;
    outPos /= outPos.w;
    outPos = uInvView * outPos;

    vOut.fragPosWS = vec3(outPos);
    EmitVertex();
}
)");

            mShadowShader = sbShadow.createProgram();
        }
    }
}
