#include "ViewerRenderer.hh"

#include <glow-extras/vector/graphics2D.hh>
#include <glow/common/non_copyable.hh>
#include <glow/common/scoped_gl.hh>

#include "Scene.hh"
#include "view.hh"

namespace
{
std::string sDefaultFontPath;

// TODO: Add viewer-global UBO with this kind of layout
// struct ShaderGlobalUbo
//{
//    glow::std140mat4x4 view;
//    glow::std140mat4x4 proj;
//    glow::std140mat4x4 invView;
//    glow::std140mat4x4 invProj;
//    glow::std140mat4x4 cleanVp;
//    glow::std140mat4x4 prevCleanVp;

//    glow::std140vec3 camPos;
//    glow::std140vec3 sunDirection;
//    glow::std140vec3 sunColor;
//};

struct AccumRenderPoolTargets
{
private:
    glow::TexturePool<glow::TextureRectangle>* const mPool;
    GLOW_RAII_CLASS(AccumRenderPoolTargets);

public:
    glow::SharedTextureRectangle shadowMap;
    glow::SharedTextureRectangle color;
    glow::SharedTextureRectangle normal;
    glow::SharedTextureRectangle colorTransparent;
    glow::SharedTextureRectangle normalTransparent;
    glow::SharedTextureRectangle depthTransparent;

    AccumRenderPoolTargets(glow::TexturePool<glow::TextureRectangle>* pool, tg::isize2 size) : mPool(pool)
    {
        shadowMap = mPool->allocAtLeast({GL_DEPTH_COMPONENT32F, {2048, 2048}});
        color = mPool->allocAtLeast({GL_RGBA16F, size});
        normal = mPool->allocAtLeast({GL_RGBA16F, size});
        colorTransparent = mPool->allocAtLeast({GL_RGBA16F, size});
        normalTransparent = mPool->allocAtLeast({GL_RGBA16F, size});
        depthTransparent = mPool->allocAtLeast({GL_DEPTH_COMPONENT32F, size});
    }

    ~AccumRenderPoolTargets()
    {
        mPool->free(&shadowMap);
        mPool->free(&color);
        mPool->free(&normal);
        mPool->free(&colorTransparent);
        mPool->free(&normalTransparent);
    }
};
}

void glow::viewer::global_set_default_font_path(const std::string& path)
{
    sDefaultFontPath = path;
    if (path.size() > 0 && path.back() != '/' && path.back() != '\\')
        sDefaultFontPath += "/";
}

glow::viewer::ViewerRenderer::ViewerRenderer()
{
    // Load shaders
    mShaderSSAO = glow::Program::createFromFile("glow-viewer/pp.ssao");
    mShaderOutline = glow::Program::createFromFile("glow-viewer/pp.outline");
    mShaderOutput = glow::Program::createFromFile("glow-viewer/pp.output");
    mShaderBackground = glow::Program::createFromFile("glow-viewer/pp.bg");
    mShaderGround = glow::Program::createFromFile("glow-viewer/pp.ground");
    mShaderAccum = glow::Program::createFromFile("glow-viewer/pp.accum");
    mShaderShadow = glow::Program::createFromFile("glow-viewer/pp.shadow");

    // Load meshes
    mMeshQuad = glow::geometry::make_quad();

    // Create framebuffers
    mFramebuffer = Framebuffer::create();
    mFramebufferColor = Framebuffer::create();
    mFramebufferSSAO = Framebuffer::create();
    mFramebufferOutput = Framebuffer::create();
    mFramebufferShadow = Framebuffer::create();
    mFramebufferShadowSoft = Framebuffer::create();

    // Register global fonts
    auto const& globalFonts = detail::internal_global_get_fonts();
#ifdef GLOW_EXTRAS_DEFAULT_FONTS
    mVectorRenderer.loadFontFromFile("sans", sDefaultFontPath + "FiraSans-Regular.ttf");
    mVectorRenderer.loadFontFromFile("mono", sDefaultFontPath + "FiraMono-Regular.ttf");
#endif
    for (auto const& font : globalFonts)
        mVectorRenderer.loadFontFromFile(font.first, font.second);
}

void glow::viewer::ViewerRenderer::beginFrame(tg::color3 const& clearColor)
{
    // set up reverse-Z depth test (1 is near, 0 is far)
    // see http://www.reedbeta.com/blog/depth-precision-visualized/
    // see https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    mIsCurrentFrameFullyConverged = true;
}

void glow::viewer::ViewerRenderer::endFrame(float approximateRenderTime)
{
    // restore default OpenGL conventions
    glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

    if (mIsCurrentFrameFullyConverged)
    {
        // If all subviews rendered this frame were converged, the approximate render time is meaningless
        // To prevent lags on the next "wake-up" draw, cap sample counts to the minimum
        mSSAOSamples = 12;
        mShadowSamplesPerFrame = 1;
    }
    else if (approximateRenderTime > 0.f)
    {
        auto multiply = [&](float mult) {
            mSSAOSamples *= mult;
            mShadowSamplesPerFrame *= mult;
        };

        auto add = [&](int val) {
            mSSAOSamples += val;
            mShadowSamplesPerFrame += val;
        };

        if (approximateRenderTime < 5)
            multiply(2);
        else if (approximateRenderTime < 10)
            add(1);
        else if (approximateRenderTime > 15)
            multiply(.5f);
        else if (approximateRenderTime > 13)
            add(-1);

        mSSAOSamples = tg::clamp(mSSAOSamples, 12, 256);
        mShadowSamplesPerFrame = tg::clamp(mShadowSamplesPerFrame, 1, 32);
    }

    texturePoolRect.cleanUp();
    texturePool2D.cleanUp();
}

void glow::viewer::ViewerRenderer::maximizeSamples()
{
    mShadowSamplesPerFrame = 32;
    mSSAOSamples = 256;
}

void glow::viewer::ViewerRenderer::renderSubview(tg::isize2 const & res, tg::ipos2 const& offset, SubViewData& subViewData, Scene const& scene, glow::viewer::CameraController& cam)
{
    if (scene.clearAccumulation)
    {
        subViewData.clearAccumBuffer();
        subViewData.clearShadowMap();
    }

    auto const& renderables = scene.getRenderables();
    auto const& boundingInfo = scene.getBoundingInfo();

    // pass lambdas
    auto performOutput = [&]() {
        // output
        {
            GLOW_SCOPED(debugGroup, "output");
            auto shader = mShaderOutput->use();
            shader["uTexOutput"] = subViewData.targetOutput;
            shader["uAccumCnt"] = subViewData.accumCount;
            shader["uViewportOffset"] = offset;
            shader["uDebugPixels"] = scene.enableScreenshotDebug;
            mMeshQuad->bind().draw();
        }

        // glow-extras-vector overlay
        {
            for (auto const& r : renderables)
                r->renderOverlay(&mVectorRenderer, res);

            mVectorImage.clear();
            {
                auto g = graphics(mVectorImage);

                auto const col_fg = scene.enablePrintMode ? tg::color3::black : tg::color3::white;
                auto const col_bg = scene.enablePrintMode ? tg::color3::white : tg::color3::black;

                std::string_view name;
                auto has_unique_name = true;
                for (auto const& r : renderables)
                {
                    if (r->name().empty())
                        continue;

                    if (!name.empty())
                        has_unique_name = false;
                    name = r->name();
                }

                if (!name.empty() && has_unique_name)
                {
                    auto f = glow::vector::font2D("sans", 24);
                    auto const y = res.height - 8.f;
                    f.blur = 4;
                    g.text({8, y}, name, f, col_bg);
                    f.blur = 1;
                    g.text({8, y}, name, f, col_bg);
                    f.blur = 0;
                    g.text({8, y}, name, f, col_fg);
                }
            }
            mVectorRenderer.render(mVectorImage, res.width, res.height);
        }
    };

    // allocate pool targets
    auto const targets = AccumRenderPoolTargets(&texturePoolRect, res);

    // adjust camera size
    cam.resize(res.width, res.height);

    glClearDepthf(0.0f);
    GLOW_SCOPED(depthFunc, GL_GREATER);

    auto camPos = cam.getPosition();
    auto view = cam.computeViewMatrix();

    // accumulation and jittering
    auto dis = 0.0f;
    for (auto x = 0; x < 3; ++x)
        for (auto y = 0; y < 3; ++y)
            dis += tg::abs(view[x][y] - subViewData.lastView[x][y]);
    if (dis > 0.01f || distance(subViewData.lastPos, camPos) > boundingInfo.diagonal / 5000.f)
    {
        // Camera changed beyond epsilon, reset accumulation
        subViewData.clearAccumBuffer();

        subViewData.lastView = view;
        subViewData.lastPos = camPos;
    }
    else // clip to last
    {
        view = subViewData.lastView;
        camPos = subViewData.lastPos;
    }

    // early out if too many samples, and no infinite accumulation configured
    if (subViewData.ssaoSampleCount > mMinSSAOCnt && subViewData.accumCount > mMinAccumCnt && !scene.infiniteAccumulation)
    {
        performOutput();
        return;
    }
    else
    {
        mIsCurrentFrameFullyConverged = false;
    }

    // compute sun
    auto sunPos = boundingInfo.center
                  + tg::vec3::unit_y * (0.5f * (boundingInfo.aabb.max.y - boundingInfo.aabb.min.y) + mSunOffsetFactor * boundingInfo.diagonal);
    if (distance(boundingInfo.center, sunPos) < boundingInfo.diagonal / 10000)
        sunPos += tg::vec3::unit_y * tg::max(1.0f, distance_to_origin(sunPos) / 50);

    auto groundY = (boundingInfo.aabb.min.y - 1e-4f) - mGroundOffsetFactor * boundingInfo.diagonal;

    auto const groundShadowAabb = tg::aabb3(tg::pos3(boundingInfo.center.x, groundY, boundingInfo.center.z) - tg::vec3(1, 0, 1) * boundingInfo.diagonal * 1,
                                            tg::pos3(boundingInfo.center.x, groundY, boundingInfo.center.z) + tg::vec3(1, 0, 1) * boundingInfo.diagonal * 1);

    for (auto _ = 0; _ < mShadowSamplesPerFrame && subViewData.shadowSampleCount < mMinShadowCnt; ++_)
    {
        auto sunPosJitter = sunPos;
        if (subViewData.shadowSampleCount > 0)
        {
            auto v = uniform_vec(mRng, tg::ball3::unit) * boundingInfo.diagonal * mSunScaleFactor / 3.f;
            v.y = 0;
            sunPosJitter += v;
        }
        auto sunDir = normalize(boundingInfo.center - sunPosJitter);
        auto sunFov = 1_deg;
        if (volume(boundingInfo.aabb) > 0)
            for (auto x : {0, 1})
                for (auto y : {0, 1})
                    for (auto z : {0, 1})
                    {
                        auto s = boundingInfo.aabb.max - boundingInfo.aabb.min;
                        auto p = boundingInfo.aabb.min;
                        p.x += s.x * x;
                        p.y += s.y * y;
                        p.z += s.z * z;
                        auto pd = normalize(p - sunPosJitter);
                        sunFov = tg::max(tg::acos(dot(sunDir, pd)) * 2, sunFov);
                    }

        auto sunView = tg::look_at(sunPosJitter, boundingInfo.center, tg::vec3::unit_x);
        auto sunProj = tg::perspective_reverse_z(sunFov, 1.0f, cam.getNearPlane());

        // draw shadow map
        {
            GLOW_SCOPED(debugGroup, "Draw shadow map");
            auto fb = mFramebufferShadow->bind();
            fb.attachDepth(targets.shadowMap);

            GLOW_SCOPED(enable, GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);

            auto const shadowMapBiasedRes = SubViewData::shadowMapSize / 2;
            RenderInfo info{sunView, sunProj, sunPos, shadowMapBiasedRes, tg::pos3::zero, subViewData.shadowSampleCount};

            for (auto const& r : renderables)
                r->renderShadow(info);
        }

        // accum soft shadow map
        {
            GLOW_SCOPED(debugGroup, "Accumulate soft shadow map");
            auto fb = mFramebufferShadowSoft->bind();
            fb.attachColor("fShadow", subViewData.shadowMapSoft);

            if (subViewData.shadowSampleCount == 0)
            {
                GLOW_SCOPED(clearColor, 0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            GLOW_SCOPED(enable, GL_BLEND);
            GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE);

            auto shader = mShaderShadow->use();
            shader["uSunView"] = sunView;
            shader["uSunProj"] = sunProj;
            shader["uGroundShadowMin"] = groundShadowAabb.min;
            shader["uGroundShadowMax"] = groundShadowAabb.max;
            shader["uShadowMap"] = targets.shadowMap;
            mMeshQuad->bind().draw();
        }

        ++subViewData.shadowSampleCount;
    }
    // update mipmaps
    subViewData.shadowMapSoft->bind().generateMipmaps();

    auto const ssaoEnabled = bool(scene.ssaoPower > 0.f);

    // accumulate multiple frames per frame
    for (auto _ = 0; _ < mAccumPerFrame; ++_)
    {
        // jittering
        auto jitter_x = uniform(mRng, -1.0f, 1.0f);
        auto jitter_y = uniform(mRng, -1.0f, 1.0f);
        if (subViewData.accumCount == 0)
            jitter_x = jitter_y = 0;
        auto proj = tg::translation(tg::vec3(jitter_x / res.width, jitter_y / res.height, 0)) * cam.computeProjMatrix();

        // main rendering
        {
            GLOW_SCOPED(debugGroup, "main render");
            auto fb = mFramebuffer->bind();
            fb.attachColor("fColor", targets.color);
            fb.attachColor("fNormal", targets.normal);
            fb.attachDepth(subViewData.targetDepth);

            GLOW_SCOPED(enable, GL_DEPTH_TEST);
            GLOW_SCOPED(clearColor, 0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // bg
            {
                GLOW_SCOPED(disable, GL_DEPTH_TEST);
                GLOW_SCOPED(disable, GL_CULL_FACE);
                auto shader = mShaderBackground->use();
                shader["uInnerColor"] = tg::vec3(scene.bgColorInner);
                shader["uOuterColor"] = tg::vec3(scene.bgColorOuter);
                shader["uPrintMode"] = scene.enablePrintMode;
                mMeshQuad->bind().draw();
            }

            // renderjobs
            {
                RenderInfo info{view, proj, sunPos, res, cam.getPosition(), subViewData.accumCount};
                for (auto const& r : renderables)
                    r->renderForward(info);
            }
        }

        // ground
        {
            GLOW_SCOPED(debugGroup, "ground");
            auto fb = mFramebuffer->bind();

            GLOW_SCOPED(enable, GL_BLEND);
            GLOW_SCOPED(enable, GL_DEPTH_TEST);
            GLOW_SCOPED(depthMask, GL_FALSE);
            GLOW_SCOPED(depthFunc, GL_ALWAYS);
            GLOW_SCOPED(disable, GL_CULL_FACE);
            GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            auto shader = mShaderGround->use();
            shader["uProj"] = proj;
            shader["uView"] = view;
            shader["uInvProj"] = inverse(proj);
            shader["uInvView"] = inverse(view);
            shader["uGroundY"] = groundY;
            shader["uCamPos"] = camPos;
            shader["uMeshDiag"] = boundingInfo.diagonal;
            shader["uMeshCenter"] = glm::vec3(boundingInfo.center.x, boundingInfo.center.y, boundingInfo.center.z);
            shader["uShadowStrength"] = scene.enableShadows ? mShadowStrength : 0.f;
            shader["uGroundShadowMin"] = groundShadowAabb.min;
            shader["uGroundShadowMax"] = groundShadowAabb.max;
            shader["uShadowSamples"] = float(subViewData.shadowSampleCount);
            shader["uShadowMapSoft"] = subViewData.shadowMapSoft;
            shader["uShowGrid"] = scene.enableGrid;
            shader["uTexDepth"] = subViewData.targetDepth;
            mMeshQuad->bind().draw();
        }

        // transparencies
        {
            GLOW_SCOPED(debugGroup, "transparency");

            auto fb = mFramebuffer->bind();
            fb.attachColor("fColor", targets.colorTransparent);
            fb.attachColor("fNormal", targets.normalTransparent);
            fb.attachDepth(targets.depthTransparent);

            GLOW_SCOPED(enable, GL_DEPTH_TEST);
            GLOW_SCOPED(clearColor, 0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // renderjobs
            {
                RenderInfo info{view, proj, sunPos, res, cam.getPosition(), subViewData.accumCount};
                for (auto const& r : renderables)
                    r->renderTransparent(info);
            }
        }

        // outline
        if (scene.enableOutlines)
        {
            GLOW_SCOPED(debugGroup, "outline");
            auto fb = mFramebufferColor->bind();
            fb.attachColor("fColor", targets.color);
            GLOW_SCOPED(disable, GL_DEPTH_TEST);
            GLOW_SCOPED(enable, GL_BLEND);
            GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            auto shader = mShaderOutline->use();
            shader["uTexDepth"] = subViewData.targetDepth;
            shader["uTexNormal"] = targets.normal;
            shader["uNearPlane"] = cam.getNearPlane();
            shader["uNormalThreshold"] = mNormalThreshold;
            shader["uInvProj"] = inverse(proj);
            shader["uInvView"] = inverse(view);
            shader["uCamPos"] = cam.getPosition();
            shader["uViewportOffset"] = offset;
            shader["uDepthThreshold"] = mDepthThresholdFactor * boundingInfo.diagonal / 50.f;
            mMeshQuad->bind().draw();
        }

        // ssao
        if (ssaoEnabled)
        {
            GLOW_SCOPED(debugGroup, "ssao");
            auto fb = mFramebufferSSAO->bind();
            fb.attachColor("fSSAO", subViewData.targetSsao);

            if (subViewData.ssaoSampleCount == 0)
            {
                GLOW_SCOPED(clearColor, 0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            GLOW_SCOPED(enable, GL_BLEND);
            GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE);

            auto shader = mShaderSSAO->use();
            shader["uTexDepth"] = subViewData.targetDepth;
            shader["uTexNormal"] = targets.normal;
            shader["uView"] = view;
            shader["uProj"] = proj;
            shader["uInvProj"] = inverse(proj);
            shader["uScreenSize"] = tg::size2(res);
            shader["uRadius"] = scene.ssaoRadius * boundingInfo.diagonal / 30.f;
            shader["uSeed"] = uint32_t(mRng());
            shader["uSamples"] = int(mSSAOSamples);
            shader["uViewportOffset"] = offset;

            mMeshQuad->bind().draw();

            subViewData.ssaoSampleCount += mSSAOSamples;
        }
        else
        {
            // Jump to min + 1 to allow early outs
            subViewData.ssaoSampleCount = mMinSSAOCnt + 1;
        }

        // accum
        {
            GLOW_SCOPED(debugGroup, "accum");
            auto fb = mFramebufferOutput->bind();
            fb.attachColor("fOutput", subViewData.targetOutput);

            auto shader = mShaderAccum->use();

            shader["uTexColor"] = targets.color;
            shader["uTexSSAO"] = subViewData.targetSsao;
            shader["uTexDepth"] = subViewData.targetDepth;
            shader["uTexColorTransparent"] = targets.colorTransparent;
            shader["uTexDepthTransparent"] = targets.depthTransparent;

            shader["uAccumCnt"] = subViewData.accumCount;
            shader["uSSAOSamples"] = subViewData.ssaoSampleCount;
            shader["uEnableSSAO"] = ssaoEnabled;
            shader["uSSAOPower"] = scene.ssaoPower;

            shader["uEnableTonemap"] = scene.enableTonemap;
            shader["uTonemapExposure"] = scene.tonemapExposure;

            shader.setImage(0, subViewData.targetAccum);

            mMeshQuad->bind().draw();
        }

        ++subViewData.accumCount;
    }

    performOutput();
}
