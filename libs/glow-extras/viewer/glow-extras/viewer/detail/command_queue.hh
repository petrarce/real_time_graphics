#pragma once

#include <functional>
#include <memory>
#include <variant>
#include <vector>

#include <glow-extras/viewer/Scene.hh>
#include <glow-extras/viewer/layout.hh>
#include <glow-extras/viewer/renderables/Renderable.hh>

namespace glow::viewer::detail
{
inline auto constexpr disableGrid = [](Scene& s) { s.enableGrid = false; };
inline auto constexpr disableShadow = [](Scene& s) { s.enableShadows = false; };
inline auto constexpr enablePrintMode = [](Scene& s) { s.enablePrintMode = true; };
inline auto constexpr enableGrid = [](Scene& s) { s.enableGrid = true; };

struct global_settings
{
    bool imguiDarkMode = false;                          ///< Whether or not imgui is shown in a dark theme
    int subviewMargin = 0;                               ///< The margin in pixels between subviews
    tg::color3 subviewMarginColor = tg::color3(0, 0, 0); ///< Color between subviews
    bool headlessScreenshot = false;                     ///< Whether to, on run, take a screenshot and immediately close
    std::string screenshotFilename = "viewer_screen.png";
    tg::ivec2 screenshotResolution = tg::ivec2(3840, 2160);
    int screenshotAccumulationCount = 64;
};

struct command
{
    enum class instruction : uint8_t
    {
        BeginSubview,
        EndSubview,
        AddRenderjob,
        ModifyScene,
        ModifyLayout,
        InteractiveSubview
    };

    using scene_modifier_func_t = std::function<void(Scene&)>;
    using interactive_func_t = std::function<void(float)>;

    instruction const instr;
    std::variant<layout::settings, SharedRenderable, scene_modifier_func_t, interactive_func_t> data;
    command(instruction i) : instr(i) {}

public:
    // == Command creators ==
    static command addRenderjob(SharedRenderable renderable)
    {
        command res{instruction::AddRenderjob};
        res.data = std::move(renderable);
        return res;
    }

    static command modifyLayout(layout::settings&& settings)
    {
        command res{instruction::ModifyLayout};
        res.data = std::move(settings);
        return res;
    }

    static command beginSubview() { return command{instruction::BeginSubview}; }
    static command endSubview() { return command{instruction::EndSubview}; }

    static command interactive(std::function<void(float)> lambda)
    {
        command res{instruction::InteractiveSubview};
        res.data = std::move(lambda);
        return res;
    }

    static command sceneNoGrid()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.enableGrid = false; };
        return res;
    }

    static command sceneNoShadow()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.enableShadows = false; };
        return res;
    }

    static command scenePrintMode()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.enablePrintMode = true; };
        return res;
    }

    static command sceneNoOutline()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.enableOutlines = false; };
        return res;
    }

    static command sceneInfiniteAccumulation()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.infiniteAccumulation = true; };
        return res;
    }

    static command sceneBackgroundColor(tg::color3 inner, tg::color3 outer)
    {
        command res{instruction::ModifyScene};
        res.data = [inner, outer](Scene& s) {
            s.bgColorInner = inner;
            s.bgColorOuter = outer;
        };
        return res;
    }

    static command sceneBackgroundColor(tg::color3 col) { return sceneBackgroundColor(col, std::move(col)); }

    static command sceneSsaoPower(float power)
    {
        command res{instruction::ModifyScene};
        res.data = [power](Scene& s) { s.ssaoPower = power; };
        return res;
    }

    static command sceneSsaoRadius(float radius)
    {
        command res{instruction::ModifyScene};
        res.data = [radius](Scene& s) { s.ssaoRadius = radius; };
        return res;
    }

    static command sceneTonemapping(float exposure)
    {
        command res{instruction::ModifyScene};
        res.data = [exposure](Scene& s) {
            s.enableTonemap = true;
            s.tonemapExposure = exposure;
        };
        return res;
    }

    static command sceneCameraOrientation(tg::angle azimuth, tg::angle altitude, float distance)
    {
        command res{instruction::ModifyScene};
        res.data = [azimuth, altitude, distance](Scene& s) {
            s.customCameraOrientation = true;
            s.cameraAzimuth = azimuth;
            s.cameraAltitude = altitude;
            s.cameraDistance = distance;
        };
        return res;
    }

    static command sceneCameraTransform(tg::pos3 pos, tg::pos3 target)
    {
        command res{instruction::ModifyScene};
        res.data = [pos, target](Scene& s) {
            s.customCameraPosition = true;
            s.cameraPosition = pos;
            s.cameraTarget = target;
        };
        return res;
    }

    static command sceneClearAccum()
    {
        command res{instruction::ModifyScene};
        res.data = [](Scene& s) { s.clearAccumulation = true; };
        return res;
    }
};

using command_queue = std::vector<command>;
using shared_command_queue = std::shared_ptr<command_queue>;

// Lazily creates the top-level command queue
void command_queue_lazy_init();
// Submits a command into the currently active command queue
void submit_command(command&& c);
// Called on the last command of any queue
void on_last_command();

void set_ui_darkmode(bool active);
void set_subview_margin(int margin);
void set_subview_margin_color(tg::color3 color);
void set_headless_screenshot(tg::ivec2 resolution, int accum, std::string const& filename);

// Returns true if the given command list contains one or more interactive instructions (and whose tree therefore must be rebuilt each frame)
bool is_interactive(command_queue const& commands);

// Builds a layout tree from a given list of commands
// deltaTime required to call interactive lambdas
void create_layout_tree(layout::tree_node& rootNode, command_queue const& commands, float deltaTime, bool allowInteractiveExecute = true);
}
