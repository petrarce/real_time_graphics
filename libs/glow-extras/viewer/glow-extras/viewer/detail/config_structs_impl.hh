#pragma once

#include <typed-geometry/types/color.hh>

namespace glow::viewer
{
struct no_grid_t
{
};
struct no_shadow_t
{
};
struct print_mode_t
{
};
struct no_outline_t
{
};
struct transparent_t
{
};
struct opaque_t
{
};
struct no_fresnel_t
{
};
struct infinite_accumulation_t
{
};

struct dark_ui_t
{
    bool active = true;
    dark_ui_t operator()(bool a) const { return {a}; }
};
struct clear_accumulation_t
{
    bool active = true;
    clear_accumulation_t operator()(bool a) const { return {a}; }
};
struct subview_margin_t
{
    int pixels = 0;
    tg::color3 color = tg::color3(0, 0, 0);
    subview_margin_t operator()(int px) const { return {px, tg::color3(0, 0, 0)}; }
    subview_margin_t operator()(tg::color3 col) const { return {1, col}; }
    subview_margin_t operator()(int px, tg::color3 col) const { return {px, col}; }
};
struct tonemap_exposure_t
{
    float exposure = 1.f;
    tonemap_exposure_t operator()(float exposure) const { return {exposure}; }

};
struct backface_culling_t
{
    bool active = true;
    backface_culling_t operator()(bool a) const { return {a}; }
};
}
