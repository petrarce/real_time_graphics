#pragma once

#include <functional>
#include <utility>

#include <glow/common/nodiscard.hh>
#include <glow/common/non_copyable.hh>

#include "Scene.hh"
#include "macros.hh"
#include "renderables/Renderable.hh"

// the three important API headers:
#include "configure.hh"
#include "detail/command_queue.hh"
#include "make_renderable.hh"

// not really needed by often used:
#include <glow-extras/colors/color.hh>
#include "materials/materials.hh"
#include "objects/objects.hh"

/**
 * One-size-fits-most function for visualizing meshes, properties, ...
 *
 * view(...): creates a view and shows it
 *
 * See viewer/Readme.md for advanced usages
 */

namespace glow
{
namespace viewer
{
namespace detail
{
struct raii_view_closer;
}

/// creates an empty 3D view
GLOW_NODISCARD detail::raii_view_closer view();

/// creates a 3D view, adds `obj` as renderable and calls v->configure(renderable, arg) for each arg in args
template <class ObjT, class... Args>
detail::raii_view_closer view(ObjT const& obj, Args const&... args);

/// creates an empty view with column layout
/// Usage:
///   auto c = viewer::columns();
///   c.view(pos, ...);
///   c.view(pos, ...);
///   ...
///   // show on end-of-scope
GLOW_NODISCARD detail::raii_view_closer columns();

/// creates an empty view with row layout
/// Usage:
///   auto r = viewer::rows();
///   r.view(pos, ...);
///   r.view(pos, ...);
///   ...
///   // show on end-of-scope
GLOW_NODISCARD detail::raii_view_closer rows();

/// creates an empty view with grid layout
/// Usage:
///   auto g = viewer::grid(2, 3);
///   g.view(pos, ...);
///   g.view(pos, ...);
///   ...
///   // show on end-of-scope
GLOW_NODISCARD detail::raii_view_closer grid(int cols, int rows);
GLOW_NODISCARD detail::raii_view_closer grid(float aspect = 1.0f); // aspect = w / h

// ======== Advanced usage ==========

void global_init();

/// Register a vector2d font that will be available to all viewer renderers created from now on
void global_add_vec2d_font(std::string const& name, std::string const& path);

/// sets the path used for default fonts (default "")
void global_set_default_font_path(std::string const& path);

/// Clear all globally registered vector2d fonts
void global_clear_vec2d_fonts();

/// Returns the current view matrix of the current subview
/// NOTE: only works inside interactive regions
tg::mat4 const& current_view_matrix();
/// Returns the current proj matrix of the current subview
/// NOTE: only works inside interactive regions
tg::mat4 const& current_proj_matrix();

// ======= Implementation ========

namespace detail
{
#if 0
// shows view on scope end
struct raii_view
{
    GLOW_RAII_CLASS(raii_view);

    // Bool value: is container
    // static std::vector<bool> sIsSubviewContainerStack;

public:
    explicit raii_view(bool isContainer)
    { /* sIsSubviewContainerStack.push_back(isContainer);*/
    }
    raii_view(raii_view&&) noexcept {}
    ~raii_view()
    {
        // TG_ASSERT(!sIsSubviewContainerStack.empty() && "corrupted subview stack");

        // remove self from subview stack
        // sIsSubviewContainerStack.pop_back();
        submit_command(command{command::instruction::EndSubview});

        if (/*sIsSubviewContainerStack.empty()*/ 0) // last one? do something TODO
        {
        }
        // mView->show();
        // else // otherwise, attach
        // sSubViewStack.back()->addSubView(mView);
    }
};
#endif

struct raii_config
{
    GLOW_RAII_CLASS(raii_config);

    using config_func = std::function<void(Renderable&)>;
    static std::vector<config_func> sConfigStack;

public:
    raii_config(std::vector<config_func> const& funcs) : mNumConfigs(int(funcs.size()))
    {
        sConfigStack.insert(sConfigStack.end(), funcs.begin(), funcs.end());
    }
    ~raii_config()
    {
        TG_ASSERT(sConfigStack.size() >= unsigned(mNumConfigs) && "corrupted subview stack");
        sConfigStack.resize(sConfigStack.size() - unsigned(mNumConfigs));
    }

    template <class T>
    static config_func packConfigFunc(T const& arg)
    {
        return config_func{[arg](Renderable& r) { ::glow::viewer::configure(r, arg); }};
    }

private:
    int const mNumConfigs;
};

struct raii_view_closer
{
private:
    bool const mMustCloseSubview; ///< If false, this view is inactive and integrates into the parent view

    bool isValid = true;

public:
    GLOW_RAII_CLASS(raii_view_closer);
    static std::vector<bool> sStackIsContainer;

    // Regular ctor
    explicit raii_view_closer(bool active) : mMustCloseSubview(active)
    {
        // No subview starts out as a container initially
        sStackIsContainer.push_back(false);
    }

    raii_view_closer(raii_view_closer&& rhs) noexcept : mMustCloseSubview(rhs.mMustCloseSubview) { rhs.isValid = false; }
    ~raii_view_closer()
    {
        if (!isValid)
            return;

        TG_ASSERT(!sStackIsContainer.empty() && "corrupted subview stack");

        // remove self from subview stack
        sStackIsContainer.pop_back();

        if (mMustCloseSubview)
            submit_command(command::endSubview());

        if (sStackIsContainer.empty())
        {
            on_last_command();
        }
    }

    static void convertCurrentSubviewToLayout(layout::settings&& settings)
    {
        TG_ASSERT(!sStackIsContainer.empty() && "corrupted subview stack");
        sStackIsContainer.back() = true;
        submit_command(command::modifyLayout(std::move(settings)));
    }

    raii_view_closer view() const { return ::glow::viewer::view(); }
    template <class ObjT, class... Args>
    raii_view_closer view(ObjT const& obj, Args const&... args) const
    {
        return ::glow::viewer::view(obj, args...);
    }

    raii_view_closer columns() { return ::glow::viewer::columns(); }
    raii_view_closer rows() { return ::glow::viewer::rows(); }
    raii_view_closer grid(int cols, int rows) { return ::glow::viewer::grid(cols, rows); }

public:
    /// Tag to construct a raii_view_closer that does nothing
    struct blank_tag
    {
    };

    explicit raii_view_closer(blank_tag) : mMustCloseSubview(false), isValid(false) {}
};

inline void initAndBeginSubview()
{
    command_queue_lazy_init();
    submit_command(command::beginSubview());
}


inline raii_view_closer getOrCreateView()
{
    // if current view is not container, it will be used to place renderables in it
    if (!detail::raii_view_closer::sStackIsContainer.empty() && !detail::raii_view_closer::sStackIsContainer.back())
    {
        // no operation, this is implicit in the command queue, we just don't begin another subview
        // the view closer has to do nothing
        return raii_view_closer{false};
    }

    // otherwise a new subview is created
    initAndBeginSubview();
    // the view closer has to close this subview again
    return raii_view_closer{true};
}

} // namespace detail

template <class... Args>
GLOW_NODISCARD detail::raii_config config(Args const&... args)
{
    std::vector<detail::raii_config::config_func> funcs;
    funcs.reserve(sizeof...(args));

    int appendHelper[] = {0, (funcs.push_back(detail::raii_config::packConfigFunc(args)), 0)...};
    (void)appendHelper;

    return detail::raii_config(funcs);
}

#define GLOW_VIEWER_CONFIG(...) auto GLOW_MACRO_JOIN(_glow_viewer_scoped_config_, __COUNTER__) = ::glow::viewer::config(__VA_ARGS__)

inline detail::raii_view_closer view()
{
    auto v = detail::getOrCreateView();
    return v;
}

template <class ObjT, class... Args>
detail::raii_view_closer view(ObjT const& obj, Args const&... args)
{
    auto v = detail::getOrCreateView();

    auto r = make_renderable(obj);
    submit_command(detail::command::addRenderjob(r));

    for (auto const& cf : detail::raii_config::sConfigStack)
        cf(*r);

    // A compile error here means configure(v, s, r, arg) was not found for one of your args
    // see configure.hh
    int a[] = {0, (configure(*r, args), 0)...};
    (void)a; // suppress unused warning

    return v;
}

inline detail::raii_view_closer columns() { return grid(-1, 1); }

inline detail::raii_view_closer rows() { return grid(1, -1); }

inline detail::raii_view_closer grid(int cols, int rows)
{
    auto v = detail::getOrCreateView();
    detail::raii_view_closer::convertCurrentSubviewToLayout({cols, rows});
    return v;
}

inline detail::raii_view_closer grid(float aspect)
{
    auto v = detail::getOrCreateView();
    detail::raii_view_closer::convertCurrentSubviewToLayout({-1, -1, aspect});
    return v;
}

inline detail::raii_view_closer nothing() { return detail::raii_view_closer{detail::raii_view_closer::blank_tag{}}; }

inline detail::raii_view_closer interactive(std::function<void(float)> func)
{
    // Never integrates, always a new top-level subview
    detail::initAndBeginSubview();
    detail::submit_command(detail::command::interactive(std::move(func)));
    return detail::raii_view_closer{true};
}

// A view that clears its accumulation each time
template <class... Args>
detail::raii_view_closer view_cleared(Args&&... args)
{
    return ::glow::viewer::view(std::forward<Args>(args)..., clear_accumulation);
}

// Clears all accumulation of the current topmost view
// Only functional inside ::interactive
inline void view_clear_accumulation() { detail::submit_command(detail::command::sceneClearAccum()); }


namespace detail
{
/// Internal use only, returns all currently registered fonts, {name, path}
std::vector<std::pair<std::string, std::string>> const& internal_global_get_fonts();
}
}
}

// insert into namespaces so ADL works
// e.g. view(pos) works without using any namespace (pos is polymesh::vertex_attribute<glm::vec3>)
GLOW_VIEWER_INJECT_FUNCTION(glow::viewer::view);

namespace gv = glow::viewer;
