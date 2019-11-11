#include "command_queue.hh"

#include <glow-extras/viewer/ViewerApp.hh>
#include <glow-extras/viewer/view.hh>
#include <glow/util/AsyncTextureLoader.hh>

namespace
{
std::vector<glow::viewer::detail::shared_command_queue> sCommandQueueStack;

std::vector<glow::viewer::detail::shared_command_queue> sUnusedCommandQueuePool;

glow::viewer::detail::shared_command_queue createQueue()
{
    if (!sUnusedCommandQueuePool.empty())
    {
        // From pool
        auto res = sUnusedCommandQueuePool.back();
        sUnusedCommandQueuePool.pop_back();
        res->clear();
        return res;
    }
    else
    {
        // New
        auto res = std::make_shared<glow::viewer::detail::command_queue>();
        res->reserve(25u);
        return res;
    }
}

void poolQueue(glow::viewer::detail::shared_command_queue&& queue) { sUnusedCommandQueuePool.push_back(std::move(queue)); }

glow::viewer::detail::global_settings sGlobalSettings{};

} // anon namespace


void glow::viewer::detail::command_queue_lazy_init()
{
    if (TG_UNLIKELY(sCommandQueueStack.empty()))
        sCommandQueueStack.push_back(createQueue());
}


void glow::viewer::detail::submit_command(glow::viewer::detail::command&& c) { sCommandQueueStack.back()->push_back(c); }

void glow::viewer::detail::on_last_command()
{
    if (sCommandQueueStack.size() == 1)
    {
        // This is the last global command queue, run viewer app with the top level queue

        // Lazily init viewer globals
        global_init();

        ViewerApp app(sCommandQueueStack.back(), sGlobalSettings);

        try
        {
            app.run();
        }
        catch (ViewerApp::TerminateException&)
        {
            // Special Shift + ESC termination
            glow::AsyncTextureLoader::shutdown();
            std::exit(0);
        }

        // Clear the entire command queue stack for subsequent viewer runs
        sCommandQueueStack.clear();

        // Reset global settings for subsequent viewer runs
        sGlobalSettings = global_settings{};
    }
    else
    {
        // A nested command queue has completed, do nothing
        // popping from the stack is handled at the place that is now being returned to
    }
}

bool glow::viewer::detail::is_interactive(const glow::viewer::detail::command_queue& commands)
{
    for (auto const& cmd : commands)
        if (cmd.instr == detail::command::instruction::InteractiveSubview)
            return true;

    return false;
}

void glow::viewer::detail::create_layout_tree(glow::viewer::layout::tree_node& rootNode, const glow::viewer::detail::command_queue& commands, float deltaTime, bool allowInteractiveExecute)
{
    using cmd_t = detail::command;

    layout::tree_node* currentNode = nullptr;
    for (auto const& cmd : commands)
    {
        if (cmd.instr == cmd_t::instruction::AddRenderjob)
        {
            TG_ASSERT(currentNode != nullptr);
            auto& renderable = std::get<SharedRenderable>(cmd.data);
            renderable->runLazyInit();
            currentNode->scene.add(renderable);
        }
        else if (cmd.instr == cmd_t::instruction::ModifyScene)
        {
            TG_ASSERT(currentNode != nullptr);
            std::get<cmd_t::scene_modifier_func_t>(cmd.data)(currentNode->scene);
        }
        else if (cmd.instr == cmd_t::instruction::BeginSubview)
        {
            // Begin a subview
            if (currentNode == nullptr)
            {
                // Begin the root subview
                currentNode = &rootNode;
            }
            else
            {
                // Start a new subview within the current one
                currentNode->children.push_back(layout::tree_node(currentNode));

                currentNode = &currentNode->children.back();
                continue;
            }
        }
        else if (cmd.instr == cmd_t::instruction::EndSubview)
        {
            TG_ASSERT(currentNode != nullptr);

            currentNode = currentNode->parent;
            if (currentNode == nullptr)
            {
                // Done, root subview has ended
                break;
            }
        }
        else if (cmd.instr == cmd_t::instruction::ModifyLayout)
        {
            TG_ASSERT(currentNode != nullptr);
            currentNode->layoutSettings = std::get<layout::settings>(cmd.data);
        }
        else if (TG_LIKELY(cmd.instr == cmd_t::instruction::InteractiveSubview && allowInteractiveExecute))
        {
            // Interactive subviews still use Begin/End as usual
            TG_ASSERT(currentNode != nullptr);

            // Create an inner command queue
            auto innerCommandQueue = createQueue();

            // Run the interactive lambda to fill innerCommandQueue
            {
                sCommandQueueStack.push_back(innerCommandQueue);
                std::get<cmd_t::interactive_func_t>(cmd.data)(deltaTime);
                sCommandQueueStack.pop_back();
            }

            // Recursively populate the tree with the recorded queue
            create_layout_tree(*currentNode, *innerCommandQueue, deltaTime);

            poolQueue(std::move(innerCommandQueue));
        }
    }
}

void glow::viewer::detail::set_ui_darkmode(bool active) { sGlobalSettings.imguiDarkMode = active; }
void glow::viewer::detail::set_subview_margin(int margin) { sGlobalSettings.subviewMargin = margin; }
void glow::viewer::detail::set_subview_margin_color(tg::color3 color) { sGlobalSettings.subviewMarginColor = color; }

void glow::viewer::detail::set_headless_screenshot(tg::ivec2 resolution, int accum, std::string const& filename)
{
    sGlobalSettings.headlessScreenshot = true;
    sGlobalSettings.screenshotFilename = filename;
    sGlobalSettings.screenshotAccumulationCount = accum;
    sGlobalSettings.screenshotResolution = resolution;
}
