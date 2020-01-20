#include "Assignment07.hh"

// System headers
#include <cstdint>
#include <fstream>
#include <vector>

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/profiling.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/Texture2DArray.hh>
#include <glow/objects/TextureCubeMap.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow/data/TextureData.hh>

#include <glow-extras/colors/color.hh>
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/timing/CpuTimer.hh>
#include <glow-extras/timing/GpuTimer.hh>

// ImGui
#include <imgui/imgui.h>

// GLFW
#include <GLFW/glfw3.h>

#include "FrustumCuller.hh"

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

namespace
{
float randomFloat(float minV, float maxV)
{
    return minV + (maxV - minV) * float(rand()) / float(RAND_MAX);
}
} // namespace

void Assignment07::update(float elapsedSeconds)
{
    mLightSpawnCountdown -= elapsedSeconds;

    if (mLightSpawnCountdown < 0.0f)
    {
        for (auto const& chunkPair : mWorld.chunks)
        {
            auto const& chunk = chunkPair.second;

            // Do not evaluate chunks that are further away than render distance
            // (0.5 * sqrt(3) ~ 0.9)
            float maxDist = mRenderDistance + 0.9f * CHUNK_SIZE;
            float maxDist2 = maxDist * maxDist;
            if (tg::distance_sqr(getCamera()->getPosition(), chunk->chunkCenter()) > maxDist2)
                continue;

            // Iterate over all light fountains and spawn light sources
            for (auto& lF : chunk->getActiveLightFountains())
            {
                // Center light pos in block
                tg::pos3 lFpos = tg::pos3(lF) + tg::vec3(0.5f);

                // Do not spawn if out of render distance
                if (tg::distance_sqr(lFpos, getCamera()->getPosition()) > mRenderDistance * mRenderDistance)
                    continue;

                spawnLightSource(lFpos);
            }
        }
        // Reset countdown to some random amount of seconds
        mLightSpawnCountdown = randomFloat(0.5f, 2) * 0.1f;
    }

    updateLightSources(elapsedSeconds);

    if (mFreeFlightCamera)
        mCharacter.setPosition(getCamera()->getPosition());

    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    mRuntime += elapsedSeconds;

    // generate chunks that might be visible
    mWorld.notifyCameraPosition(getCamera()->getPosition(), mRenderDistance);

    // update terrain
    mWorld.update(elapsedSeconds);

    // character
    if (!mFreeFlightCamera)
    {
        auto walkspeed = mCharacter.getMovementSpeed(mShiftPressed);

        auto movement = tg::vec3(0, 0, 0);
        if (isKeyDown(GLFW_KEY_W)) // forward
            movement.z -= walkspeed;
        if (isKeyDown(GLFW_KEY_S)) // backward
            movement.z += walkspeed;
        if (isKeyDown(GLFW_KEY_A)) // left
            movement.x -= walkspeed;
        if (isKeyDown(GLFW_KEY_D)) // right
            movement.x += walkspeed;
        if (mDoJump)
        {
            movement.y += mCharacter.getJumpSpeed(mShiftPressed);
            mDoJump = false;
        }
        auto newPos = mCharacter.update(mWorld, elapsedSeconds, movement, (tg::mat3(getCamera()->getViewMatrix())));
        getCamera()->setPosition(newPos);
        getWASDController().setCameraSpeed(0.f);
    }
    else
        getWASDController().setCameraSpeed(30.f);
}

void Assignment07::render(float elapsedSeconds)
{
    // limit GPU to 60 fps?
    setVSync(mVSync);

    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);
    if (!mBackFaceCulling)
        glDisable(GL_CULL_FACE);

    // update stats
    mStatsChunksGenerated = mWorld.chunks.size();
    for (auto i = 0; i < 4; ++i)
    {
        mStatsMeshesRendered[i] = 0;
        mStatsVerticesRendered[i] = 0;
    }

    // update camera
    getCamera()->setFarPlane(mRenderDistance);

    // build lights
    {
        std::vector<LightVertex> lightData;
        lightData.reserve(mLightSources.size());
        for (auto const& l : mLightSources)
            lightData.push_back({l.position, l.radius, l.color, l.seed});
        mLightArrayBuffer->bind().setData(lightData);
    }

    // renormalize light dir (UI might have changed it)
    mLightDir = normalize(mLightDir);

    // rendering pipeline
    {
        // Shadow Pass
        renderShadowPass();

        // Depth Pre-Pass
        renderDepthPrePass();

        // Opaque Pass
        renderOpaquePass();

        // Light Pass
        renderLightPass();

        // Transparent Pass
        renderTransparentPass();

        // Transparent Resolve
        renderTransparentResolve();

        // Output Stage
        renderOutputStage();
    }

    // update stats
    for (auto i = 0; i < 4; ++i)
        mStatsVerticesPerMesh[i] = mStatsVerticesRendered[i] == 0 ? -1 : //
                                       mStatsVerticesRendered[i] / (float)mStatsMeshesRendered[i];


    auto constexpr statUpdateRate = 1.0f;
    mLastFrameTimes[(mCurframeidx++) % accumframes] = elapsedSeconds * 1000;
    if (mLastStatUpdate < mRuntime - statUpdateRate)
    {
        mLastStatUpdate = mRuntime;
        mAvgFrameTime = 0.0f;
        for (auto t : mLastFrameTimes)
            mAvgFrameTime += t;
        mAvgFrameTime /= accumframes;
    }
}

void Assignment07::onGui()
{
    if (ImGui::Begin("RTG Blocky Assignment ++"))
    {
        if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("VSync (Limit to 60 fps)", &mVSync);
            ImGui::Checkbox("Back Face Culling", &mBackFaceCulling);
            ImGui::Checkbox("Show Wireframe (Opaque)", &mShowWireframeOpaque);
            ImGui::Checkbox("Show Wireframe (Transparent)", &mShowWireframeTransparent);
            ImGui::Checkbox("Show Wireframe (Lights)", &mShowDebugLights);
            ImGui::Checkbox("Highlight Wrong Z-Pre", &mShowWrongDepthPre);

            constexpr static int outputs = 16;
            const char* element_names[outputs]
                = {"Final Rendering",        "Opaque Depth",     "Shaded Opaque",      "G-Buffer: Albedo",
                   "G-Buffer: AO",           "G-Buffer: Normal", "G-Buffer: Metallic", "G-Buffer: Roughness",
                   "G-Buffer: Translucency", "T-Buffer: Color",  "T-Buffer: Alpha",    "T-Buffer: Distortion",
                   "T-Buffer: Blurriness",   "Shadow Cascade 0", "Shadow Cascade 1",   "Shadow Cascade 2"};
            static int current_element = (int)DebugTarget::Output;

            ImGui::Text("Output: ");
            if (ImGui::BeginCombo("##combo", element_names[current_element])) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < outputs; n++)
                {
                    bool is_selected = (current_element == n); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(element_names[n], is_selected))
                        current_element = n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            TG_ASSERT(0 <= current_element && current_element < outputs);
            mDebugOutput = (DebugTarget)current_element;
        }

        if (ImGui::CollapsingHeader("Light and Shadow", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Shadows
            ImGui::Checkbox("Shadows", &mEnableShadows);
            ImGui::Checkbox("Soft Shadows", &mSoftShadows);

            enum SMSizeElement
            {
                sm_512,
                sm_1024,
                sm_2048,
                sm_4096
            };
            const char* element_names[4] = {"512", "1024", "2048", "4096"};
            static int current_element = sm_1024;
            const char* current_element_name
                = (current_element >= 0 && current_element < 4) ? element_names[current_element] : "Unknown";
            ImGui::SliderInt("Shadow Map Size", &current_element, 0, 3, current_element_name);
            mShadowMapSize = 1 << (9 + current_element);

            ImGui::ColorEdit3("Ambient Light", &mAmbientLight.r);
            ImGui::ColorEdit3("Sun Light", &mLightColor.r);

            static float sun_angle = 45.0f;
            ImGui::SliderFloat("Sun Angle", &sun_angle, 10.0f, 170.0f);

            auto xz = tg::cos(tg::degree(sun_angle));
            mLightDir = tg::normalize(tg::vec3{0.01f * xz, tg::sin(tg::degree(sun_angle)), 0.99f * xz});

            ImGui::SliderFloat("Shadow Max Distance", &mShadowRange, 20.0f, 500.0f);
        }

        if (ImGui::CollapsingHeader("Culling", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Culling
            ImGui::SliderFloat("Render Distance", &mRenderDistance, 1.0f, 1000.0f);
            ImGui::Checkbox("Frustum Culling", &mEnableFrustumCulling);
            ImGui::Checkbox("Custom BFC", &mEnableCustomBFC);
        }

        if (ImGui::CollapsingHeader("Pipeline", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Pipeline
            ImGui::Checkbox("Background", &mDrawBackground);
            ImGui::Checkbox("Point Lights", &mEnablePointLights);
            ImGui::Checkbox("Pass: Depth-Pre", &mPassDepthPre);
            ImGui::Checkbox("Pass: Opaque", &mPassOpaque);
            ImGui::Checkbox("Pass: Transparent", &mPassTransparent);
            ImGui::Checkbox("FXAA", &mUseFXAA);
            ImGui::Checkbox("Dithering", &mUseDithering);
        }

        if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (mAvgFrameTime > 0)
                ImGui::Text("ms/frame: %d ms (%d fps)", int(tg::round(mAvgFrameTime)), int(tg::round(1000 / mAvgFrameTime)));
            else
                ImGui::Text("ms/frame: ");

            ImGui::Text("Chunks: %d", mStatsChunksGenerated);
            ImGui::Text("Z-Pre: Meshes: %d", mStatsMeshesRendered[(int)RenderPass::DepthPre]);
            ImGui::Text("Z-Pre: Vertices: %d", mStatsVerticesRendered[(int)RenderPass::DepthPre]);
            ImGui::Text("Z-Pre: Vertices / Mesh: %f", mStatsVerticesPerMesh[(int)RenderPass::DepthPre]);

            ImGui::Text("Opaque: Meshes: %d", mStatsMeshesRendered[(int)RenderPass::Opaque]);
            ImGui::Text("Opaque: Vertices: %d", mStatsVerticesRendered[(int)RenderPass::Opaque]);
            ImGui::Text("Opaque: Vertices / Mesh: %f", mStatsVerticesPerMesh[(int)RenderPass::Opaque]);

            ImGui::Text("Transparent: Meshes: %d", mStatsMeshesRendered[(int)RenderPass::Transparent]);
            ImGui::Text("Transparent: Vertices: %d", mStatsVerticesRendered[(int)RenderPass::Transparent]);
            ImGui::Text("Transparent: Vertices / Mesh: %f", mStatsVerticesPerMesh[(int)RenderPass::Transparent]);

            ImGui::Text("Shadow: Meshes: %d", mStatsMeshesRendered[(int)RenderPass::Shadow]);
            ImGui::Text("Shadow: Vertices: %d", mStatsVerticesRendered[(int)RenderPass::Shadow]);
            ImGui::Text("Shadow: Vertices / Mesh: %f", mStatsVerticesPerMesh[(int)RenderPass::Shadow]);
        }
    }
    ImGui::End();
}


void Assignment07::renderShadowPass()
{
    // ensure that sizes are correct
    updateShadowMapTexture();

    auto cam = getCamera();
    for (auto cascIdx = 0; cascIdx < SHADOW_CASCADES; ++cascIdx)
    {
        auto& cascade = mShadowCascades[cascIdx];

        // build frustum part
        cascade.minRange = mRenderDistance * (cascIdx + 0.0f) / SHADOW_CASCADES;
        cascade.maxRange = mRenderDistance * (cascIdx + 1.0f) / SHADOW_CASCADES;

        auto cInvView = inverse(cam->getViewMatrix());
        auto cInvProj = inverse(cam->getProjectionMatrix());

        auto sView = tg::look_at(tg::pos3::zero + mLightDir, tg::pos3::zero, tg::vec3::unit_y);

        auto sMin = tg::vec3(std::numeric_limits<float>::max());
        auto sMax = -sMin;

        // calculate shadow frustum
        auto const res = 8;
        for (auto nz : {-1, 1})
            for (auto iy = 0; iy < res; ++iy)
                for (auto ix = 0; ix < res; ++ix)
                {
                    auto ny = iy / (res - 1.0f) * 2 - 1;
                    auto nx = ix / (res - 1.0f) * 2 - 1;

                    auto ndc = tg::vec4(nx, ny, nz, 1.0f);
                    auto viewPos = cInvProj * ndc;
                    viewPos /= viewPos.w;
                    auto worldPos = tg::pos3(cInvView * viewPos);

                    auto dir = tg::vec3(worldPos - cam->getPosition());
                    auto dis = length(dir);
                    worldPos = cam->getPosition() + dir * (tg::clamp(dis, cascade.minRange, cascade.maxRange) / dis);

                    auto shadowViewPos = tg::vec3(sView * tg::vec4(worldPos, 1.0));

                    sMin = min(sMin, shadowViewPos);
                    sMax = max(sMax, shadowViewPos);
                }

        // elongate shadow aabb
        sMin = floor(sMin);
        sMax = ceil(sMax);
        sMin -= 1.0f;
        sMax += 1.0f;
        sMax.z = sMin.z + tg::max(mShadowRange, sMax.z - sMin.z);

        // min..max -> 0..1 -> -1..1
        auto sProj = scaling(tg::size3(1, 1, -1)) *                   // flip z for BFC
                     translation(tg::pos3(-1.0f)) *                   //
                     scaling(tg::size3(1.0f / (sMax - sMin) * 2.0)) * //
                     translation(-sMin);

        // TODO: stabilize min/max

        // set up shadow camera
        cascade.camera.setPosition(tg::pos3::zero + mLightDir);
        cascade.camera.setViewMatrix(sView);
        cascade.camera.setProjectionMatrix(sProj);
        cascade.camera.setViewportSize({mShadowMapSize, mShadowMapSize});
        mShadowViewProjs[cascIdx] = cascade.camera.getProjectionMatrix() * cascade.camera.getViewMatrix();

        // render shadowmap
        {
            auto fb = cascade.framebuffer->bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            // clear sm
            {
                GLOW_SCOPED(disable, GL_DEPTH_TEST);
                GLOW_SCOPED(disable, GL_CULL_FACE);
                auto shader = mShaderClear->use();
                shader.setUniform("uColor", tg::vec4(tg::exp(mShadowExponent)));
                mMeshQuad->bind().draw();
            }

            // render scene from light
            if (mEnableShadows)
                renderScene(&cascade.camera, RenderPass::Shadow);
        }

        // blur shadow map for soft shadows
        if (mEnableShadows && mSoftShadows)
        {
            GLOW_SCOPED(disable, GL_DEPTH_TEST);
            GLOW_SCOPED(disable, GL_CULL_FACE);
            auto vao = mMeshQuad->bind();

            // blur x
            {
                auto fb = mFramebufferShadowBlur->bind();
                auto shader = mShaderShadowBlurX->use();
                shader.setTexture("uTexture", mShadowMaps);
                shader.setUniform("uCascade", cascIdx);

                vao.draw();
            }

            mShadowBlurTarget->setMipmapsGenerated(true); // only one LOD level

            // blur y
            {
                auto fb = cascade.framebuffer->bind();
                auto shader = mShaderShadowBlurY->use();
                shader.setTexture("uTexture", mShadowBlurTarget);

                vao.draw();
            }
        }
    }
}

void Assignment07::renderDepthPrePass()
{
    /// Task 1.b
    ///
    /// Your job is to:
    ///     - bind and clear the depth-pre buffer
    ///     - set the correct depthFunc state
    ///     - render the scene as depth-pre pass
    ///
    /// Notes:
    ///     - do not render the scene if mPassDepthPre is false
    ///
    /// ============= STUDENT CODE BEGIN =============
    auto fb = mFramebufferDepthPre->bind();

    glClear(GL_DEPTH_BUFFER_BIT);

    GLOW_SCOPED(depthFunc, GL_LESS);

    if (mPassDepthPre) renderScene(getCamera().get(), RenderPass::DepthPre);
    /// ============= STUDENT CODE END =============
}

void Assignment07::renderOpaquePass()
{
    // debug: wireframe rendering
    GLOW_SCOPED(wireframe, mShowWireframeOpaque);

    /// Task 1.a
    ///
    /// Your job is to:
    ///     - bind and clear the gbuffer (color only, not depth!)
    ///     - set the correct depthFunc
    ///     - enable writing to an sRGB framebuffer
    ///     - render the scene as opaque pass (renderScene(...))
    ///
    /// Notes:
    ///     - do not render the scene if mPassOpaque is false
    ///     - getCamera().get() gives you a pointer to the current camera
    ///
    /// ============= STUDENT CODE BEGIN =============
    auto fb = mFramebufferGBuffer->bind();

    GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLOW_SCOPED(depthFunc, GL_LEQUAL);
    GLOW_SCOPED(enable, GL_FRAMEBUFFER_SRGB);

    if (mPassOpaque) renderScene(getCamera().get(), RenderPass::Opaque);
    /// ============= STUDENT CODE END =============
}

void Assignment07::renderLightPass()
{
    setUpLightShader(mShaderFullscreenLight.get(), getCamera().get());
    setUpLightShader(mShaderPointLight.get(), getCamera().get());

    /// Task 1.c - fullscreen lighting pass
    ///
    /// Your job is to:
    ///     - enable additive blending
    ///     - temporarily disable depth test and backface culling
    ///     - Perform a fullscreen lighting pass and render into the "ShadedOpaque" buffer
    ///
    /// Notes:
    ///     - do not forget to clear the color of the buffer at first (not the depth!)
    ///     - do not forget to pick the right shader
    ///     - fullscreen passes are rendered using the mMeshQuad geometry (cf. renderOutput)
    ///
    /// ============= STUDENT CODE BEGIN =============
    auto fb = mFramebufferShadedOpaque->bind();

    GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLOW_SCOPED(enable, GL_BLEND);
    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    GLOW_SCOPED(disable, GL_CULL_FACE);

    auto shader = mShaderFullscreenLight->use();

    mMeshQuad->bind().draw();

    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);
    /// ============= STUDENT CODE END =============

    // point lights
    if (mEnablePointLights)
    {
        // debug: wireframe rendering

        GLOW_SCOPED(wireframe, mShowDebugLights);

        /// Task 1.c - point lights pass
        ///
        /// Your job is to:
        ///     - render the mMeshLightSpheres geometry into the currently bound buffer
        ///
        /// Notes:
        ///     - here we need culling and depth test again
        ///     - however, we must not WRITE the depth (see OpenGL depth mask)
        ///
        /// ============= STUDENT CODE BEGIN =============
        GLOW_SCOPED(depthMask, GL_FALSE);

        auto shader = mShaderPointLight->use();

        mMeshLightSpheres->bind().draw();

        GLOW_SCOPED(depthMask, GL_TRUE);
        /// ============= STUDENT CODE END =============
    }
}

void Assignment07::renderTransparentPass()
{
    // debug: wireframe rendering
    GLOW_SCOPED(wireframe, mShowWireframeTransparent);

    /// Task 2.b
    /// Transparent Pass with Weighted, Blended OIT and Distortion
    ///
    /// Your job is to:
    ///     - clear the T-Buffer (color only)
    ///     - set the correct render state
    ///     - write all transparent objects into the T-Buffer
    ///
    /// Notes:
    ///     - we swapped revealage and accum.a so that we can use the same blend mode for all buffers
    ///       (RGB is additive, A is zero and 1-src.alpha)
    ///     - we want to render without culling and without writing depth (but with depth test)
    ///     - think about the correct clear color
    ///
    /// ============= STUDENT CODE BEGIN =============

    /// ============= STUDENT CODE END =============
}

void Assignment07::renderTransparentResolve()
{
    auto fb = mFramebufferTransparentResolve->bind();

    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    GLOW_SCOPED(disable, GL_CULL_FACE);

    setUpLightShader(mShaderTransparentResolve.get(), getCamera().get());
    auto shader = mShaderTransparentResolve->use();
    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexShadedOpaque", mTexShadedOpaque);
    shader.setTexture("uTexTBufferAccumA", mTexTBufferAccumA);
    shader.setTexture("uTexTBufferAccumB", mTexTBufferAccumB);
    shader.setTexture("uTexTBufferDistortion", mTexTBufferDistortion);

    shader.setUniform("uDrawBackground", mDrawBackground);

    mMeshQuad->bind().draw();
}

void Assignment07::renderOutputStage()
{
    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    GLOW_SCOPED(disable, GL_CULL_FACE);

    // upload shader debug info
    setUpShader(mShaderOutput.get(), getCamera().get(), RenderPass::Transparent);

    auto shader = mShaderOutput->use();
    shader.setTexture("uTexture", mTexHDRColor);
    shader.setUniform("uUseFXAA", mUseFXAA);
    shader.setUniform("uUseDithering", mUseDithering);

    // pipeline debug
    shader.setTexture("uShadowMaps", mShadowMaps);
    shader.setUniform("uShadowExponent", mShadowExponent);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexShadedOpaque", mTexShadedOpaque);

    shader.setTexture("uTexGBufferColor", mTexGBufferColor);
    shader.setTexture("uTexGBufferMatA", mTexGBufferMatA);
    shader.setTexture("uTexGBufferMatB", mTexGBufferMatB);

    shader.setTexture("uTexTBufferAccumA", mTexTBufferAccumA);
    shader.setTexture("uTexTBufferAccumB", mTexTBufferAccumB);
    shader.setTexture("uTexTBufferDistortion", mTexTBufferDistortion);

    shader.setUniform("uDebugOutput", (int)mDebugOutput);

    mMeshQuad->bind().draw();
}

void Assignment07::renderScene(camera::CameraBase* cam, RenderPass pass)
{
    // set up general purpose shaders
    switch (pass)
    {
    case RenderPass::Transparent:
        setUpShader(mShaderLineTransparent.get(), cam, pass);
        setUpShader(mShaderLightSprites.get(), cam, pass);
        break;
    default:
        break;
    }

    // render terrain
    {
        FrustumCuller culler(*cam, pass == RenderPass::Shadow);

        struct RenderJob
        {
            Program* program;
            RenderMaterial const* mat;
            VertexArray* mesh;
            float camDis;
        };

        // render jobs
        std::vector<RenderJob> jobs;

        // collect meshes per material and shader
        {
            for (auto const& chunkPair : mWorld.chunks)
            {
                auto const& chunk = chunkPair.second;

                // early out
                if (chunk->isFullyAir())
                    continue; // CAUTION: fully solid is wrong here -> top layer may have faces!

                // view-frustum culling
                if (mEnableFrustumCulling && !culler.isAabbVisible(chunk->getAabbMin(), chunk->getAabbMax()))
                    continue; // skip culled chunks

                // render distance
                if (pass != RenderPass::Shadow && !culler.isAabbInRange(chunk->getAabbMin(), chunk->getAabbMax(), mRenderDistance))
                    continue; // not in range

                for (auto const& mesh : chunk->queryMeshes())
                {
                    // check correct render pass
                    auto mat = mesh.mat.get();
                    if (!mat->opaque && pass != RenderPass::Transparent)
                        continue;
                    if (mat->opaque && (pass != RenderPass::Opaque && pass != RenderPass::Shadow && pass != RenderPass::DepthPre))
                        continue;

                    // view-frustum culling (pt. 2)
                    if (mEnableFrustumCulling && !culler.isAabbVisible(mesh.aabbMin, mesh.aabbMax))
                        continue; // skip culled meshes

                    // custom BFC
                    if (mEnableCustomBFC && mat->opaque && !culler.isFaceVisible(mesh.dir, mesh.aabbMin, mesh.aabbMax))
                        continue;

                    // render distance
                    if (pass != RenderPass::Shadow && !culler.isAabbInRange(mesh.aabbMin, mesh.aabbMax, mRenderDistance))
                        continue; // not in range

                    // create a render job for every material/mesh pair
                    Program* shader = nullptr;
                    VertexArray* vao = nullptr;
                    switch (pass)
                    {
                    case RenderPass::Shadow:
                        vao = mesh.vaoPosOnly.get();
                        shader = mShaderTerrainShadow.get();
                        break;

                    case RenderPass::DepthPre:
                        vao = mesh.vaoPosOnly.get();
                        shader = mShaderTerrainDepthPre.get();
                        break;

                    case RenderPass::Transparent:
                    case RenderPass::Opaque:
                        vao = mesh.vaoFull.get();
                        shader = mShadersTerrain[mat->shader].get();
                        break;

                    default:
                        assert(0 && "not supported");
                        break;
                    }

                    auto camDis = distance(cam->getPosition(), (mesh.aabbMin + mesh.aabbMax) / 2.0);

                    // add render job
                    jobs.push_back({shader, mat, vao, camDis});
                }
            }
        }

        // .. sort renderjobs
        {
            // sort by
            // .. shader
            // .. material
            // .. front-to-back
            std::sort(jobs.begin(), jobs.end(), [](RenderJob const& a, RenderJob const& b) {
                if (a.program != b.program)
                    return a.program < b.program;
                if (a.mat != b.mat)
                    return a.mat < b.mat;
                return a.camDis < b.camDis;
            });
        }

        // .. render per shader
        {
            auto idxShader = 0u;
            while (idxShader < jobs.size())
            {
                // set up shader
                auto program = jobs[idxShader].program;
                setUpShader(program, cam, pass);
                auto shader = program->use();

                // .. per material
                auto idxMaterial = idxShader;
                while (idxMaterial < jobs.size() && jobs[idxMaterial].program == program)
                {
                    auto mat = jobs[idxMaterial].mat;

                    // set up material
                    shader.setUniform("uMetallic", mat->metallic);
                    shader.setUniform("uReflectivity", mat->reflectivity);
                    shader.setUniform("uTranslucency", mat->translucency);
                    shader.setUniform("uTextureScale", mat->textureScale);
                    shader.setTexture("uTexAO", mat->texAO);
                    shader.setTexture("uTexAlbedo", mat->texAlbedo);
                    shader.setTexture("uTexNormal", mat->texNormal);
                    shader.setTexture("uTexHeight", mat->texHeight);
                    shader.setTexture("uTexRoughness", mat->texRoughness);

                    // .. per mesh
                    auto idxMesh = idxMaterial;
                    while (idxMesh < jobs.size() && jobs[idxMesh].mat == mat)
                    {
                        auto mesh = jobs[idxMesh].mesh;

                        // keep stats
                        mStatsMeshesRendered[(int)pass]++;
                        mStatsVerticesRendered[(int)pass] += mesh->getVertexCount();

                        mesh->bind().draw(); // render

                        // advance idx
                        ++idxMesh;
                    }

                    // advance idx
                    idxMaterial = idxMesh;
                }

                // advance idx
                idxShader = idxMaterial;
            }
        }
    }

    // mouse hit
    if (mMouseHit.hasHit && pass == RenderPass::Transparent)
    {
        drawLine(mMouseHit.hitPos, mMouseHit.hitPos + tg::vec3(mMouseHit.hitNormal) * 0.2f, {1, 1, 1}, pass);
    }

    // lights
    if (mEnablePointLights && pass == RenderPass::Transparent)
    {
        GLOW_SCOPED(disable, GL_CULL_FACE); // no culling

        auto shader = mShaderLightSprites->use();
        shader.setTexture("uTexLightSprites", mTexLightSprites);

        mMeshLightSprites->bind().draw();
    }

    // render debug box overlay
    if (mMouseHit.hasHit && pass == RenderPass::Transparent)
    {
        tg::color3 overlayColor;
        auto boxPos = mMouseHit.blockPos;
        if (mCtrlPressed)
        {
            // Remove material
            overlayColor = tg::color3::red;
        }
        else if (mShiftPressed)
        {
            // Use pipette to get material
            overlayColor = tg::color3::yellow;
        }
        else
        {
            // Place material
            overlayColor = tg::color3::green;
            boxPos += mMouseHit.hitNormal;
        }

        // draw AABB
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        drawAABB(tg::pos3(boxPos) + 0.01f, tg::pos3(boxPos + 1) - 0.01f, overlayColor, pass);
    }
}

void Assignment07::setUpShader(glow::Program* program, camera::CameraBase* cam, RenderPass pass)
{
    auto shader = program->use();

    auto view = cam->getViewMatrix();
    auto proj = cam->getProjectionMatrix();

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uViewProj", proj * view);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());

    shader.setUniform("uRuntime", (float)mRuntime);

    shader.setUniform("uShowWrongDepthPre", mShowWrongDepthPre);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setUniform("uRenderDistance", mRenderDistance);

    if (pass == RenderPass::Transparent)
    {
        shader.setTexture("uTexCubeMap", mTexSkybox);
        shader.setUniform("uLightDir", normalize(mLightDir));
        shader.setUniform("uAmbientLight", mAmbientLight);
        shader.setUniform("uLightColor", mLightColor);

        shader.setUniform("uShadowExponent", mShadowExponent);
        shader.setTexture("uShadowMaps", mShadowMaps);
        shader.setUniform("uShadowViewProjs", mShadowViewProjs);
        shader.setUniform("uShadowViewProjs[0]", mShadowViewProjs);
        shader.setUniform("uShadowPos", mShadowPos);
        shader.setUniform("uShadowRange", mShadowRange);
    }

    if (pass == RenderPass::Shadow)
    {
        shader.setUniform("uShadowExponent", mShadowExponent);
        shader.setUniform("uShadowPos", mShadowPos);
    }
}

void Assignment07::setUpLightShader(glow::Program* program, glow::camera::CameraBase* cam)
{
    auto shader = program->use();

    auto view = cam->getViewMatrix();
    auto proj = cam->getProjectionMatrix();

    shader.setUniform("uDebugLights", mShowDebugLights);

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uViewProj", proj * view);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());

    shader.setTexture("uTexCubeMap", mTexSkybox);
    shader.setUniform("uLightDir", normalize(mLightDir));
    shader.setUniform("uAmbientLight", mAmbientLight);
    shader.setUniform("uLightColor", mLightColor);
    shader.setUniform("uRenderDistance", mRenderDistance);

    shader.setUniform("uShadowExponent", mShadowExponent);
    shader.setTexture("uShadowMaps", mShadowMaps);
    shader.setUniform("uShadowViewProjs", mShadowViewProjs);
    shader.setUniform("uShadowViewProjs[0]", mShadowViewProjs);
    shader.setUniform("uShadowPos", mShadowPos);
    shader.setUniform("uShadowRange", mShadowRange);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexGBufferColor", mTexGBufferColor);
    shader.setTexture("uTexGBufferMatA", mTexGBufferMatA);
    shader.setTexture("uTexGBufferMatB", mTexGBufferMatB);
}

void Assignment07::buildLineMesh()
{
    auto ab = ArrayBuffer::create();
    ab->defineAttribute<float>("aPosition");
    ab->bind().setData(std::vector<float>({0.0f, 1.0f}));
    mMeshLine = VertexArray::create(ab, GL_LINES);
}

void Assignment07::drawLine(tg::pos3 from, tg::pos3 to, tg::color3 color, RenderPass pass)
{
    if (pass != RenderPass::Transparent)
    {
        glow::error() << "not implemented.";
        return;
    }

    auto shader = mShaderLineTransparent->use();
    shader.setUniform("uFrom", from);
    shader.setUniform("uTo", to);
    shader.setUniform("uColor", color);

    mMeshLine->bind().draw();
}

void Assignment07::drawAABB(tg::pos3 min, tg::pos3 max, tg::color3 color, RenderPass pass)
{
    if (pass != RenderPass::Transparent)
    {
        glow::error() << "not implemented.";
        return;
    }

    auto shader = mShaderLineTransparent->use();
    auto vao = mMeshLine->bind();

    shader.setUniform("uColor", color);

    for (auto dir : {0, 1, 2})
        for (auto dx : {0, 1})
            for (auto dy : {0, 1})
            {
                tg::vec3 n(dir == 0, dir == 1, dir == 2);
                tg::vec3 t(dir == 1, dir == 2, dir == 0);
                tg::vec3 b(dir == 2, dir == 0, dir == 1);

                auto s = t * dx + b * dy;
                auto e = s + n;

                shader.setUniform("uFrom", tg::mix(min, max, tg::comp3(s)));
                shader.setUniform("uTo", tg::mix(min, max, tg::comp3(e)));

                vao.draw();
            }
}

void Assignment07::spawnLightSource(tg::pos3 const& origin)
{
    LightSource ls;
    ls.position = origin;
    ls.radius = randomFloat(0.8f, 2.5f);
    ls.velocity = tg::vec3(randomFloat(-.6f, .6f), randomFloat(4.0f, 6.5f), randomFloat(-.6f, .6f));
    ls.intensity = 1.0f;
    ls.seed = rand();
    auto col = glow::colors::color::from_hsv(randomFloat(0, 360), 1, 1);
    ls.color = {col.r, col.g, col.b};
    mLightSources.push_back(ls);
}

void Assignment07::updateLightSources(float elapsedSeconds)
{
    // gravity would be too much
    float acceleration = -3.0;
    for (auto i = int(mLightSources.size()) - 1; i >= 0; --i)
    {
        auto& ls = mLightSources[i];
        ls.velocity += tg::vec3(0, acceleration, 0) * elapsedSeconds;
        ls.position += ls.velocity * elapsedSeconds;

        // Check if below terrain
        auto block = mWorld.queryBlock(tg::ipos3(ls.position + tg::vec3(0, ls.radius, 0)));
        if (!block.isAir())
        {
            mLightSources.erase(mLightSources.begin() + i);
        }
    }
}

void Assignment07::init()
{
    setGui(GlfwApp::Gui::ImGui);

    // disable built-in camera handling with left mouse button
    setUseDefaultCameraHandlingLeft(false);

    GlfwApp::init(); // Call to base GlfwApp

    auto texPath = util::pathOf(__FILE__) + "/textures/";
    auto shaderPath = util::pathOf(__FILE__) + "/shader/";
    auto meshPath = util::pathOf(__FILE__) + "/meshes/";

    // set up camera and character
    {
        auto cam = getCamera();
        cam->setPosition({12, 12, 12});
        cam->handle.setTarget(tg::pos3::zero);

        mCharacter = Character(cam->getPosition());
    }

    // load shaders
    {
        glow::info() << "Loading shaders";

        // pipeline
        mShaderOutput = Program::createFromFile(shaderPath + "pipeline/fullscreen.output");
        mShaderClear = Program::createFromFile(shaderPath + "pipeline/fullscreen.clear");
        mShaderShadowBlurX = Program::createFromFile(shaderPath + "pipeline/fullscreen.shadow-blur-x");
        mShaderShadowBlurY = Program::createFromFile(shaderPath + "pipeline/fullscreen.shadow-blur-y");

        mShaderTransparentResolve = Program::createFromFile(shaderPath + "pipeline/fullscreen.transparent-resolve");

        // lights
        mShaderFullscreenLight = Program::createFromFile(shaderPath + "pipeline/fullscreen.light");
        mShaderPointLight = Program::createFromFile(shaderPath + "pipeline/point-light");
        mShaderLightSprites = Program::createFromFile(shaderPath + "objects/light-sprite");

        // objects
        mShaderLineTransparent = Program::createFromFile(shaderPath + "objects/line.transparent");

        // terrain
        mShaderTerrainShadow = Program::createFromFile(shaderPath + "terrain/terrain.shadow");
        mShaderTerrainDepthPre = Program::createFromFile(shaderPath + "terrain/terrain.depth-pre");
        // ... more in world mat init
    }

    // shadow map
    // -> created on demand

    // rendering pipeline
    {
        // targets
        mFramebufferTargets.push_back(mTexOpaqueDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32));
        mFramebufferDepthPre = Framebuffer::create(std::vector<FramebufferAttachment>(), mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexGBufferColor = TextureRectangle::create(1, 1, GL_SRGB8_ALPHA8));
        mFramebufferTargets.push_back(mTexGBufferMatA = TextureRectangle::create(1, 1, GL_RGBA8));
        mFramebufferTargets.push_back(mTexGBufferMatB = TextureRectangle::create(1, 1, GL_RG8));
        mFramebufferGBuffer = Framebuffer::create(
            {
                {"fColor", mTexGBufferColor}, //
                {"fMatA", mTexGBufferMatA},   //
                {"fMatB", mTexGBufferMatB}    //
            },
            mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexShadedOpaque = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferShadedOpaque = Framebuffer::create({{"fColor", mTexShadedOpaque}}, mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexTBufferAccumA = TextureRectangle::create(1, 1, GL_RGBA16F));
        mFramebufferTargets.push_back(mTexTBufferAccumB = TextureRectangle::create(1, 1, GL_R16F));
        mFramebufferTargets.push_back(mTexTBufferDistortion = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTBuffer = Framebuffer::create(
            {
                {"fAccumA", mTexTBufferAccumA},        //
                {"fAccumB", mTexTBufferAccumB},        //
                {"fDistortion", mTexTBufferDistortion} //
            },
            mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexHDRColor = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTransparentResolve = Framebuffer::create({{"fColor", mTexHDRColor}});
    }

    // load textures
    {
        glow::info() << "Loading textures";

        mTexSkybox = TextureCubeMap::createFromData(TextureData::createFromFileCube( //
            texPath + "bg/posx.jpg",                                                 //
            texPath + "bg/negx.jpg",                                                 //
            texPath + "bg/posy.jpg",                                                 //
            texPath + "bg/negy.jpg",                                                 //
            texPath + "bg/posz.jpg",                                                 //
            texPath + "bg/negz.jpg",                                                 //
            ColorSpace::sRGB));

        mTexLightSprites = Texture2D::createFromFile(texPath + "lights.jpg", ColorSpace::sRGB);

        // terrain textures are loaded in world::init (materials)
    }

    // create geometry
    {
        glow::info() << "Loading geometry";

        mMeshQuad = geometry::Quad<>().generate();
        mMeshCube = geometry::Cube<>().generate();
        buildLineMesh();
    }

    // create light geometry
    {
        mLightArrayBuffer = ArrayBuffer::create(LightVertex::attributes());
        mLightArrayBuffer->setDivisor(1); // instancing

        mMeshLightSpheres = geometry::UVSphere<>().generate();
        mMeshLightSpheres->bind().attach(mLightArrayBuffer);

        mMeshLightSprites = geometry::Quad<>().generate();
        mMeshLightSprites->bind().attach(mLightArrayBuffer);
    }

    // init world
    {
        glow::info() << "Init world";

        mWorld.init();

        // create terrain shaders
        for (auto const& mat : mWorld.materialsOpaque)
            for (auto const& rmat : mat.renderMaterials)
                createTerrainShader(rmat->shader);

        for (auto const& mat : mWorld.materialsTranslucent)
            for (auto const& rmat : mat.renderMaterials)
                createTerrainShader(rmat->shader);
    }
}

void Assignment07::getMouseRay(tg::pos3& pos, tg::vec3& dir) const
{
    auto mp = getMousePosition();
    auto x = mp.x;
    auto y = mp.y;

    auto cam = getCamera();
    tg::vec3 ps[2];
    auto i = 0;
    for (auto d : {0.5f, -0.5f})
    {
        tg::vec4 v{x / float(getWindowWidth()) * 2 - 1, 1 - y / float(getWindowHeight()) * 2, d * 2 - 1, 1.0};

        v = tg::inverse(cam->getProjectionMatrix()) * v;
        v /= v.w;
        v = tg::inverse(cam->getViewMatrix()) * v;
        ps[i++] = tg::vec3(v);
    }

    pos = cam->getPosition();
    dir = normalize(ps[0] - ps[1]);
}

void Assignment07::updateViewRay()
{
    // calculate mouse ray
    tg::pos3 pos;
    tg::vec3 dir;
    getMouseRay(pos, dir);

    mMouseHit = mWorld.rayCast(pos, dir);
}

void Assignment07::createTerrainShader(std::string const& name)
{
    if (mShadersTerrain.count(name))
        return; // already added

    glow::info() << "Loading material shader " << name << ".fsh";
    auto shaderPath = util::pathOf(__FILE__) + "/shader/terrain/";
    auto program = Program::createFromFile(shaderPath + "terrain." + name);
    mShadersTerrain[name] = program;
}

void Assignment07::updateShadowMapTexture()
{
    if (mShadowMaps && (int)mShadowMaps->getWidth() == mShadowMapSize)
        return; // already done

    glow::info() << "Creating " << mShadowMapSize << " x " << mShadowMapSize << " shadow maps";

    mShadowCascades.resize(SHADOW_CASCADES);
    auto shadowDepth = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32, 1);
    mShadowMaps = Texture2DArray::createStorageImmutable(mShadowMapSize, mShadowMapSize, SHADOW_CASCADES, GL_R32F, 1);
    mShadowMaps->bind().setMinFilter(GL_LINEAR);                     // no mip-maps
    mShadowMaps->bind().setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); // clamp

    for (auto i = 0; i < SHADOW_CASCADES; ++i)
    {
        auto& cascade = mShadowCascades[i];

        // attach i-th layer of mShadowMaps
        cascade.framebuffer = Framebuffer::create({{"fShadow", mShadowMaps, 0, i}}, shadowDepth);
    }

    // shadow blur texture/target
    mShadowBlurTarget = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_R32F, 1);
    mFramebufferShadowBlur = Framebuffer::create({{"fShadow", mShadowBlurTarget}});
}

bool Assignment07::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    updateViewRay();

    setUseDefaultCameraHandling(true);

    if (mMouseHit.hasHit && action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        setUseDefaultCameraHandling(false);

        bool modified = false;
        auto bPos = mMouseHit.blockPos;
        auto blockMat = mMouseHit.block.mat;
        if (mods & GLFW_MOD_CONTROL)
        {
            mWorld.queryBlockMutable(bPos).mat = 0;
            modified = true;
            // glow::info() << "Removing material " << int(mMouseHit.block.mat) << " at " << bPos;
        }
        else if (mods & GLFW_MOD_SHIFT)
        {
            mCurrentMaterial = blockMat;
            auto matName = mWorld.getMaterialFromIndex(blockMat)->name;
            glow::info() << "Selected material is now " << matName;
        }
        else // no modifier -> add material
        {
            bPos += mMouseHit.hitNormal;
            mWorld.queryBlockMutable(bPos).mat = mCurrentMaterial;
            modified = true;
            // glow::info() << "Adding material " << int(mCurrentMaterial) << " at " << bPos;
        }

        // trigger mesh rebuilding
        if (modified)
            mWorld.markDirty(bPos, 1);

        return true;
    }

    setUseDefaultCameraHandling(true);

    return false;
}

bool Assignment07::onMousePosition(double x, double y)
{
    if (GlfwApp::onMousePosition(x, y))
        return true;

    updateViewRay();

    return false;
}

bool Assignment07::onKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
        mShiftPressed = action != GLFW_RELEASE;

    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        mCtrlPressed = action != GLFW_RELEASE;

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        mFreeFlightCamera = !mFreeFlightCamera;
        glow::info() << "Set camera to " << (mFreeFlightCamera ? "free" : "first-person");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        mDoJump = true;


    updateViewRay();

    return false;
}

void Assignment07::onResize(int w, int h)
{
    GlfwApp::onResize(w, h);

    // resize framebuffer textures
    for (auto const& t : mFramebufferTargets)
        t->bind().resize(w, h);
}
