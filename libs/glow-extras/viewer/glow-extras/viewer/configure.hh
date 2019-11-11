#pragma once

#include <vector>

#include "fwd.hh"
#include "traits.hh"

#include <glow-extras/colors/color.hh>

#include "detail/MeshAttribute.hh"
#include "detail/config_structs_impl.hh"

#include "renderables/GeometricRenderable.hh"

// view configuration
// is called from the variadic view(obj, ...) function

namespace glow::viewer
{
// Config settings
struct background_color
{
    tg::color3 inner;
    tg::color3 outer;
    constexpr explicit background_color(tg::color3 in, tg::color3 out) : inner(std::move(in)), outer(std::move(out)) {}
    constexpr explicit background_color(tg::color3 c) : inner(c), outer(std::move(c)) {}
};
struct ssao_power
{
    float power;
    constexpr explicit ssao_power(float s) : power(s) {}
};
struct ssao_radius
{
    float radius;
    constexpr explicit ssao_radius(float r) : radius(r) {}
};
struct camera_orientation
{
    tg::angle azimuth;
    tg::angle altitude;
    float distance;

    constexpr explicit camera_orientation(tg::angle azimuth, tg::angle altitude, float distance = -1.f)
      : azimuth(azimuth), altitude(altitude), distance(distance)
    {
    }
};
struct camera_transform
{
    tg::pos3 pos;
    tg::pos3 target;
    constexpr explicit camera_transform(tg::pos3 pos, tg::pos3 target) : pos(pos), target(target) {}
};

struct headless_screenshot
{
    std::string filename;
    tg::ivec2 resolution;
    int accumulationCount;

    explicit headless_screenshot(tg::ivec2 resolution = tg::ivec2(3840, 2160), int accumulationCount = 64, std::string_view filename = "viewer_screen.png")
      : filename(filename), resolution(resolution), accumulationCount(accumulationCount)
    {
    }
};

// Config tags
inline auto constexpr no_grid = no_grid_t{};
inline auto constexpr no_shadow = no_shadow_t{};
inline auto constexpr print_mode = print_mode_t{};
inline auto constexpr no_outline = no_outline_t{};
inline auto constexpr no_ssao = ssao_power(0.f);
inline auto constexpr transparent = transparent_t{};
inline auto constexpr opaque = opaque_t{};
inline auto constexpr no_fresnel = no_fresnel_t{};
inline auto constexpr clear_accumulation = clear_accumulation_t{};
inline auto constexpr dark_ui = dark_ui_t{};
inline auto constexpr subview_margin = subview_margin_t{};
inline auto constexpr infinite_accumulation = infinite_accumulation_t{};
inline auto constexpr tonemap_exposure = tonemap_exposure_t{};
inline auto constexpr backface_culling = backface_culling_t{};

// passthrough configures for scene modification commands
void configure(Renderable&, no_grid_t);
void configure(Renderable&, no_shadow_t);
void configure(Renderable&, print_mode_t);
void configure(Renderable&, no_outline_t);
void configure(Renderable&, infinite_accumulation_t);
void configure(Renderable&, dark_ui_t b);
void configure(Renderable&, subview_margin_t b);
void configure(Renderable&, clear_accumulation_t b);
void configure(Renderable&, tonemap_exposure_t const& b);
void configure(Renderable&, background_color const& b);
void configure(Renderable&, ssao_power const& b);
void configure(Renderable&, ssao_radius const& b);
void configure(Renderable&, camera_orientation const& b);
void configure(Renderable&, camera_transform const& b);
void configure(Renderable&, headless_screenshot const& b);

/// overrides the transform of the given renderable
void configure(Renderable& r, tg::mat4 const& transform);
void configure(Renderable& r, glm::mat4 const& transform);

/// names the renderable
void configure(Renderable& r, char const* name);
void configure(Renderable& r, std::string_view name);
void configure(Renderable& r, std::string const& name);

/// add renderjob
void configure(Renderable&, SharedRenderable const& rj);

/// global color of the renderable
void configure(GeometricRenderable& r, glow::colors::color const& c);
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, Color const& c);

/// per-primitive color of the renderable
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, polymesh::vertex_attribute<Color> const& c);
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, polymesh::face_attribute<Color> const& c);
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, polymesh::edge_attribute<Color> const& c);
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, polymesh::halfedge_attribute<Color> const& c);
template <class Color, class = std::enable_if_t<detail::is_color3_like<Color> || detail::is_color4_like<Color>>>
void configure(GeometricRenderable& r, std::vector<Color> const& c);

/// mapping from data to color
void configure(GeometricRenderable& r, ColorMapping const& cm);

/// textured mesh
void configure(GeometricRenderable& r, Texturing const& t);

/// render modes
void configure(GeometricRenderable& r, transparent_t);
void configure(GeometricRenderable& r, opaque_t);
void configure(GeometricRenderable& r, no_fresnel_t);

/// culling
void configure(GeometricRenderable& r, backface_culling_t b);

// ================== Implementation ==================
template <class Color, class>
void configure(GeometricRenderable& r, Color const& c)
{
    if constexpr (detail::is_color4_like<Color>)
    {
        configure(r, glow::colors::color(c.r, c.g, c.b, c.a));
        if (c.a < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
    }
    else
        configure(r, glow::colors::color(c.r, c.g, c.b));
}

template <class Color, class>
void configure(GeometricRenderable& r, polymesh::vertex_attribute<Color> const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));

    if constexpr (detail::is_color4_like<Color>)
        if (c.min([](auto&& v) { return v.a; }) < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}

template <class Color, class>
void configure(GeometricRenderable& r, polymesh::face_attribute<Color> const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));

    if constexpr (detail::is_color4_like<Color>)
        if (c.min([](auto&& v) { return v.a; }) < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}

template <class Color, class>
void configure(GeometricRenderable& r, polymesh::halfedge_attribute<Color> const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));

    if constexpr (detail::is_color4_like<Color>)
        if (c.min([](auto&& v) { return v.a; }) < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}

template <class Color, class>
void configure(GeometricRenderable& r, polymesh::edge_attribute<Color> const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));

    if constexpr (detail::is_color4_like<Color>)
        if (c.min([](auto&& v) { return v.a; }) < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}

template <class Color, class>
void configure(GeometricRenderable& r, std::vector<Color> const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));

    if constexpr (detail::is_color4_like<Color>)
        if (tg::min_element(c, [](auto&& v) { return v.a; }) < 1)
            r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}
}
