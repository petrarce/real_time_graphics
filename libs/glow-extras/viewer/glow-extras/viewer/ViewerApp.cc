#include "ViewerApp.hh"

#include <iomanip>
#include <sstream>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imguizmo.h>

#include <glow-extras/debugging/imgui-util.hh>
#include <glow/common/scoped_gl.hh>

#include "view.hh"

using namespace glow;
using namespace glow::viewer;

namespace
{
thread_local tg::mat4 sCurrentViewMatrix;
thread_local tg::mat4 sCurrentProjMatrix;
thread_local bool sIsViewerActive = false;

std::string toCppString(float v)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6);
    if (v == 0.f || !tg::is_finite(v))
        ss << "0.f";
    else
        ss << v << "f";
    return ss.str();
}
std::string toCppString(tg::pos3 const& v)
{
    std::stringstream ss;
    ss << "tg::pos3(" << toCppString(v.x) << ", " << toCppString(v.y) << ", " << toCppString(v.z) << ")";
    return ss.str();
}
std::string toCppString(tg::angle32 v)
{
    std::stringstream ss;
    ss << "tg::degree(" << toCppString(v.degree()) << ")";
    return ss.str();
}
}

tg::mat4 const& glow::viewer::current_view_matrix()
{
    TG_ASSERT(sIsViewerActive && "viewer not currently active");
    return sCurrentViewMatrix;
}
tg::mat4 const& glow::viewer::current_proj_matrix()
{
    TG_ASSERT(sIsViewerActive && "viewer not currently active");
    return sCurrentProjMatrix;
}

SubViewData& ViewerApp::getOrCreateSubViewData(tg::ipos2 start, tg::isize2 size)
{
    auto it = mSubViewDataCache.find(start);
    if (TG_LIKELY(it != mSubViewDataCache.end()))
        return (*it).second;
    else
    {
        auto it2 = mSubViewDataCache.emplace(std::piecewise_construct, std::forward_as_tuple(start), std::forward_as_tuple(size.width, size.height, &mRenderer));
        return it2.first->second;
    }
}

ViewerApp::ViewerApp(glow::viewer::detail::shared_command_queue queue, glow::viewer::detail::global_settings const& settings)
  : GlfwApp(Gui::ImGui), mStaticCommandQueue(std::move(queue)), mIsInteractive(detail::is_interactive(*mStaticCommandQueue)), mSettings(settings)
{
    mLayoutNodes.reserve(8);
}

void ViewerApp::init()
{
    setDumpTimingsOnShutdown(false);
    setUsePipeline(false);
    setUsePipelineConfigGui(false);
    setUseDefaultCamera(false);
    setUseDefaultCameraHandling(false);
    setCacheWindowSize(true);
    setWarnOnFrameskip(false);
    setTitle("Viewer");

    GlfwApp::init();

    // create camera (before initial layout)
    mCamera = CameraController::create();
    mCamera->s.HorizontalFov = 60_deg;

    // Initial layout
    forceInteractiveExecution(0.f, getWindowSize());

    // adapt cam to scene
    resetCameraToScene();

    debugging::applyGlowImguiTheme(mSettings.imguiDarkMode);

    // Dummy render
    render(0.f);
    mSubViewDataCache.clear();

    if (mSettings.headlessScreenshot)
    {
        renderScreenshot(mSettings.screenshotFilename, mSettings.screenshotResolution.x, mSettings.screenshotResolution.y, mSettings.screenshotAccumulationCount);
        requestClose();
    }
}

void ViewerApp::update(float)
{
    // empty for now
}


void ViewerApp::updateCamera(float dt)
{
    float dRight = 0;
    float dUp = 0;
    float dFwd = 0;
    float speed = mKeyShift ? 5 : 1;
    speed *= mKeyCtrl ? 0.2f : 1;

    dRight += int(mKeyD) * speed;
    dRight -= int(mKeyA) * speed;
    dFwd += int(mKeyW) * speed;
    dFwd -= int(mKeyS) * speed;
    dUp += int(mKeyE) * speed;
    dUp -= int(mKeyQ) * speed;

    mCamera->moveCamera(dRight, dFwd, dUp, dt);

    mCamera->update(dt);

    setCursorMode(mMouseRight ? glow::glfw::CursorMode::Disabled : glow::glfw::CursorMode::Normal);
}

void ViewerApp::updateLayoutNodes(float dt, tg::isize2 size)
{
    ImGuizmo::SetRect(0, 0, size.width, size.height);
    layout::tree_node rootNode;

    // for now: only works correctly with one view
    sIsViewerActive = true;
    sCurrentViewMatrix = mCamera->computeViewMatrix();
    sCurrentProjMatrix = mCamera->computeProjMatrix();

    // calculate tree, always execute interactives
    detail::create_layout_tree(rootNode, *mStaticCommandQueue, dt, true);
    sIsViewerActive = false;

    // calculate linear nodes
    mLayoutNodes.clear();
    layout::build_linear_nodes(rootNode, {0, -1}, size, mLayoutNodes, mSettings.subviewMargin);
}

void ViewerApp::forceInteractiveExecution(float dt, tg::isize2 size)
{
    // Imgui state has to be alive so the interactive lambda doesn't crash
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    updateLayoutNodes(dt, size);
    ImGui::EndFrame();
}

void ViewerApp::renderFrame(int width, int height, bool maximizeSamples)
{
#ifdef GLOW_EXTRAS_OPENGL_DEBUG_GROUPS
    GLOW_SCOPED(debugGroup, "Render Viewer");
#endif
    mRenderer.beginFrame(mSettings.subviewMarginColor);

    if (maximizeSamples)
        mRenderer.maximizeSamples();

    sIsViewerActive = true;
    for (auto& node : mLayoutNodes)
    {
#ifdef GLOW_EXTRAS_OPENGL_DEBUG_GROUPS
        GLOW_SCOPED(debugGroup, "SubView Render");
#endif

        auto start = node.start;
        auto const& size = node.size;
        start.y = height - start.y - size.height - 1;

        // render
        {
            GLOW_SCOPED(viewport, start, size);

            // debug settings
            node.scene.enableScreenshotDebug = mShowNonTransparentPixels;

            // metadata
            node.scene.viewMatrix = mCamera->computeViewMatrix();
            node.scene.projMatrix = mCamera->computeProjMatrix();
            sCurrentViewMatrix = node.scene.viewMatrix;
            sCurrentProjMatrix = node.scene.projMatrix;

            mRenderer.renderSubview(size, start, getOrCreateSubViewData(start, size), node.scene, *mCamera);
        }
    }

    mRenderer.endFrame(maximizeSamples ? 0.f : getLastGpuTimeMs());
    sIsViewerActive = false;
}

void ViewerApp::resetCameraToScene()
{
    for (auto const& node : mLayoutNodes)
    {
        auto const& bounding = node.scene.getBoundingInfo();
        mCamera->setupMesh(bounding.diagonal, bounding.center);

        if (node.scene.customCameraOrientation)
            mCamera->setOrbit(node.scene.cameraAzimuth, node.scene.cameraAltitude, node.scene.cameraDistance);
        if (node.scene.customCameraPosition)
            mCamera->setTransform(node.scene.cameraPosition, node.scene.cameraTarget);
    }

    mCamera->clipCamera();
}

void ViewerApp::render(float dt)
{
    updateCamera(dt);
    renderFrame(getWindowWidth(), getWindowHeight());
}

bool ViewerApp::onMousePosition(double x, double y)
{
    if (GlfwApp::onMousePosition(x, y))
        return true;

    auto dx = x - mLastX;
    auto dy = y - mLastY;

    // camera movement
    {
        auto minS = double(glm::min(getWindowWidth(), getWindowHeight()));
        auto relDx = dx / minS;
        auto relDy = dy / minS;

        if (mMouseRight)
            mCamera->fpsStyleLookaround(float(relDx), float(relDy));
        else if (mMouseLeft)
            mCamera->targetLookaround(float(relDx), float(relDy));
    }

    mLastX = x;
    mLastY = y;

    return false;
}

bool ViewerApp::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && clickCount >= 2 && mods == 0)
        glow::warning() << "Unimplemented";
    // mCamera->focusOnSelectedPoint(x, getWindowHeight() - y - 1, mFramebuffer);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        mMouseLeft = true;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        mMouseLeft = false;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        mMouseRight = true;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        mMouseRight = false;


    return false;
}

bool ViewerApp::onKey(int key, int scancode, int action, int mods)
{
    if (GlfwApp::onKey(key, scancode, action, mods))
        return true;

    // Close
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            if (input().isKeyDown(GLFW_KEY_LEFT_SHIFT))
            {
                // Terminate on Shift + ESC
                throw TerminateException();
            }
            else
            {
                // Close on ESC
                requestClose();
            }
            return true;
        }

        if (key == GLFW_KEY_F2)
            renderScreenshot(mScreenshotFile);

        if (key == GLFW_KEY_F3)
            resetCameraToScene();
    }

    // Camera handling
    {
        // Numpad+- - camera speed
        if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
        {
            mCamera->changeCameraSpeed(1);
            return true;
        }
        if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
        {
            mCamera->changeCameraSpeed(-1);
            return true;
        }

        // Numpad views:
        if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS)
        {
            mCamera->rotate(0, 1);
            return true;
        }
        if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS)
        {
            mCamera->rotate(0, -1);
            return true;
        }
        if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS)
        {
            mCamera->rotate(1, 0);
            return true;
        }
        if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS)
        {
            mCamera->rotate(-1, 0);
            return true;
        }
        if (key == GLFW_KEY_KP_0 && action == GLFW_PRESS)
        {
            mCamera->resetView();
            return true;
        }
        if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS)
        {
            mCamera->setOrbit({1, 0, 0}, {0, 1, 0});
            return true;
        }
        if (key == GLFW_KEY_KP_9 && action == GLFW_PRESS)
        {
            mCamera->setOrbit({0, 1, 0}, {1, 0, 0});
            return true;
        }
        if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS)
        {
            mCamera->setOrbit({0, -1, 0}, {-1, 0, 0});
            return true;
        }

        // movement
        if (key == GLFW_KEY_A && action == GLFW_PRESS)
            mKeyA = true;
        if (key == GLFW_KEY_A && action == GLFW_RELEASE)
            mKeyA = false;

        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
            mKeyShift = true;
        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
            mKeyShift = false;

        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
            mKeyCtrl = true;
        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
            mKeyCtrl = false;

        if (key == GLFW_KEY_D && action == GLFW_PRESS)
            mKeyD = true;
        if (key == GLFW_KEY_D && action == GLFW_RELEASE)
            mKeyD = false;

        if (key == GLFW_KEY_S && action == GLFW_PRESS)
            mKeyS = true;
        if (key == GLFW_KEY_S && action == GLFW_RELEASE)
            mKeyS = false;

        if (key == GLFW_KEY_W && action == GLFW_PRESS)
            mKeyW = true;
        if (key == GLFW_KEY_W && action == GLFW_RELEASE)
            mKeyW = false;

        if (key == GLFW_KEY_Q && action == GLFW_PRESS)
            mKeyQ = true;
        if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
            mKeyQ = false;

        if (key == GLFW_KEY_E && action == GLFW_PRESS)
            mKeyE = true;
        if (key == GLFW_KEY_E && action == GLFW_RELEASE)
            mKeyE = false;
    }


    return false;
}

bool ViewerApp::onMouseScroll(double sx, double sy)
{
    if (GlfwApp::onMouseScroll(sx, sy))
        return true;

    mCamera->zoom(float(sy));

    return false;
}

void ViewerApp::onResize(int w, int h)
{
    mSubViewDataCache.clear();
    if (!mIsInteractive)
    {
        // Non-interactive viewers dont rebuild the layout each frame, do it now
        updateLayoutNodes(0.f, {w, h});
        sIsViewerActive = false;
    }
}

void ViewerApp::onGui()
{
    // this gets called each frame, before ::render

    // if interactive, perform layouting now so the interactive lambda can use imgui
    if (mIsInteractive)
        updateLayoutNodes(getCurrentDeltaTime(), getWindowSize());

    // perform viewer-global imgui
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::PushID("glow_viewer_menu");

        ImGui::MenuItem("Viewer", nullptr, false, false);

        if (ImGui::BeginMenu("Actions"))
        {
            if (ImGui::MenuItem("Take Screenshot", "F2"))
                renderScreenshot(mScreenshotFile);

            if (ImGui::MenuItem("Reset Camera", "F3"))
                resetCameraToScene();

            if (ImGui::MenuItem("Quit", "Esc"))
                requestClose();

            if (ImGui::MenuItem("Quit All", "Shift + Esc"))
                throw TerminateException();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::MenuItem("Screenshot Tool", nullptr, &mShowScreenshotTool);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Info"))
        {
            auto const camOrientation = mCamera->getSphericalCoordinates();
            ImGui::Text("Cam Orientation: Azimuth: %.3f, Altitude: %.3f", camOrientation.comp0.degree(), camOrientation.comp1.degree());
            ImGui::Text("Cam Distance: %.3f", mCamera->getTargetDistance());
            auto const camPos = mCamera->getPosition();
            ImGui::Text("Cam Position: x: %.3f, y: %.3f, z: %.3f", camPos.x, camPos.y, camPos.z);
            auto const camTarget = mCamera->getTargetPos();
            ImGui::Text("Cam Target: x: %.3f, y: %.3f, z: %.3f", camTarget.x, camTarget.y, camTarget.z);

            if (ImGui::BeginMenu("Print cam configuration"))
            {
                if (ImGui::MenuItem("Orientation"))
                {
                    glow::info() << "Cam config:\nGLOW_VIEWER_CONFIG(glow::viewer::camera_orientation(" << toCppString(camOrientation.comp0) << ", "
                                 << toCppString(camOrientation.comp1) << ", " << toCppString(mCamera->getTargetDistance()) << "));";
                }

                if (ImGui::MenuItem("Transform (Full)"))
                {
                    glow::info() << "Cam config:\nGLOW_VIEWER_CONFIG(glow::viewer::camera_transform(" << toCppString(mCamera->getPosition()) << ", "
                                 << toCppString(mCamera->getTargetPos()) << "));";
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Overlays"))
        {
            if (ImGui::MenuItem("Profiling", "F9"))
                toggleProfilingOverlay();

            if (ImGui::MenuItem("OpenGL Log", "F10"))
                toggleDebugOverlay();


            ImGui::EndMenu();
        }

        ImGui::PopID();
        ImGui::EndMainMenuBar();
    }

    // screenshot tool
    if (mShowScreenshotTool)
    {
        if (ImGui::Begin("Screenshot Tool", &mShowScreenshotTool, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushID("screenshot_tool");

            ImGui::InputText("Output Path", &mScreenshotFile);

            if (ImGui::Button("Render Screenshot"))
            {
                if (mScreenshotCustomRes)
                    renderScreenshot(mScreenshotFile, mScreenshotWidth, mScreenshotHeight, mScreenshotAccumulationCount);
                else
                    renderScreenshot(mScreenshotFile, -1, -1, mScreenshotAccumulationCount);
            }

            ImGui::Separator();

            ImGui::Checkbox("Show non-transparent pixels", &mShowNonTransparentPixels);

            ImGui::SliderInt("Accumulation Count", &mScreenshotAccumulationCount, 1, 256);

            ImGui::Checkbox("Custom Resolution", &mScreenshotCustomRes);
            if (ImGui::SliderInt("Screenshot Width", &mScreenshotWidth, 1, 16000))
                mScreenshotCustomRes = true;
            if (ImGui::SliderInt("Screenshot Height", &mScreenshotHeight, 1, 16000))
                mScreenshotCustomRes = true;

            ImGui::PopID();
        }
        ImGui::End();
    }

    // perform imgui for each renderable
    sIsViewerActive = true;
    for (auto const& node : mLayoutNodes)
    {
        ImGuizmo::SetRect(node.start.x, node.start.y, node.size.width, node.size.height);
        for (auto const& r : node.scene.getRenderables())
        {
            sCurrentViewMatrix = node.scene.viewMatrix;
            sCurrentProjMatrix = node.scene.projMatrix;
            r->onGui();
        }
    }
    ImGuizmo::SetRect(0, 0, getWindowWidth(), getWindowHeight());
    sIsViewerActive = false;
}

void ViewerApp::renderScreenshot(const std::string& filename, int w, int h, int accumulationCount)
{
    auto customRes = w > 0 && h > 0;
    auto screenW = customRes ? w : getWindowWidth();
    auto screenH = customRes ? h : getWindowHeight();

    auto target = glow::TextureRectangle::create(screenW, screenH, GL_RGB8);
    auto temporaryFbo = glow::Framebuffer::create("fOut", target);

    if (customRes)
    {
        onResize(screenW, screenH);
        if (mIsInteractive)
            updateLayoutNodes(0.f, {screenW, screenH});
    }

    {
        auto fbo = temporaryFbo->bind();
        for (auto _ = 0; _ < accumulationCount; ++_)
            renderFrame(screenW, screenH, true);
    }

    target->bind().writeToFile(filename);
    glow::info() << "Saved screenshot as " << filename;

    if (customRes)
    {
        onResize(getWindowWidth(), getWindowHeight());
        if (mIsInteractive)
            updateLayoutNodes(0.f, getWindowSize());
    }
}
