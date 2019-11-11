#pragma once

#include <unordered_map>

#include "fwd.hh"

#include <typed-geometry/std/hash.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include <glow-extras/vector/backend/opengl.hh>

#include "CameraController.hh"
#include "RenderInfo.hh"
#include "ViewerRenderer.hh"
#include "detail/command_queue.hh"
#include "layout.hh"

namespace glow
{
namespace viewer
{
class ViewerApp final : public glfw::GlfwApp
{
public:
    struct TerminateException : std::exception
    {
        const char* what() const noexcept { return "Global Viewer Temination"; }
    };
    // input
private:
    bool mMouseLeft = false;
    bool mMouseRight = false;
    bool mKeyShift = false;
    bool mKeyCtrl = false;
    bool mKeyA = false;
    bool mKeyD = false;
    bool mKeyS = false;
    bool mKeyW = false;
    bool mKeyQ = false;
    bool mKeyE = false;
    double mLastX = 0;
    double mLastY = 0;

    // UI
private:
    bool mShowScreenshotTool = false;
    bool mShowNonTransparentPixels = false;
    bool mScreenshotCustomRes = false;
    int mScreenshotWidth = 3840;
    int mScreenshotHeight = 2160;
    std::string mScreenshotFile = "viewer_screen.png";
    int mScreenshotAccumulationCount = 1;

    // gfx
private:
    ViewerRenderer mRenderer;
    SharedCameraController mCamera;

    /// The static part of the command queue, this does not change during application run
    detail::shared_command_queue const mStaticCommandQueue;
    /// Whether mStaticCommandQueue contains interactive parts. If false, the layout does not require a rebuild each frame
    bool const mIsInteractive;
    detail::global_settings const mSettings;

    /// A subview data cache, keys are x/y upper left corner
    std::unordered_map<tg::ipos2, SubViewData> mSubViewDataCache;

    /// The linear layout nodes
    std::vector<layout::linear_node> mLayoutNodes;

    SubViewData& getOrCreateSubViewData(tg::ipos2 start, tg::isize2 size);

private:
    void updateCamera(float dt);
    void updateLayoutNodes(float dt, tg::isize2 size);
    void forceInteractiveExecution(float dt, tg::isize2 size);
    void renderFrame(int w, int h, bool maximizeSamples = false);
    void resetCameraToScene();

public:
    ViewerApp(detail::shared_command_queue queue, detail::global_settings const& settings);

protected:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    bool onMousePosition(double x, double y) override;
    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    bool onMouseScroll(double sx, double sy) override;

    void onResize(int w, int h) override;
    void onGui() override;

private:
    void renderScreenshot(std::string const& filename, int w = -1, int h = -1, int accumulationCount = 1);
};
}
}
