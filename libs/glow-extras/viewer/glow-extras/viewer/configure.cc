#include "configure.hh"

#include <typed-geometry/tg.hh>

#include "Scene.hh"
#include "detail/command_queue.hh"
#include "renderables/GeometricRenderable.hh"
#include "renderables/MeshRenderable.hh"
#include "renderables/PointRenderable.hh"
#include "renderables/Renderable.hh"

namespace glow::viewer
{
void configure(Renderable& r, tg::mat4 const& transform) { r.transform(transform); }
void configure(Renderable& r, glm::mat4 const& transform)
{
    tg::mat4 m;
    std::memcpy(data_ptr(m), value_ptr(transform), sizeof(m));
    r.transform(m);
}

void configure(GeometricRenderable& r, glow::colors::color const& c)
{
    r.addAttribute(detail::make_mesh_attribute("aColor", c));
    if (c.a < 1)
        r.setRenderMode(GeometricRenderable::RenderMode::Transparent);
}
void configure(GeometricRenderable& r, ColorMapping const& cm) { r.setColorMapping(cm); }
void configure(GeometricRenderable& r, Texturing const& t) { r.setTexturing(t); }

void configure(Renderable&, no_grid_t) { detail::submit_command(detail::command::sceneNoGrid()); }
void configure(Renderable&, no_shadow_t) { detail::submit_command(detail::command::sceneNoShadow()); }
void configure(Renderable&, print_mode_t) { detail::submit_command(detail::command::scenePrintMode()); }
void configure(Renderable&, no_outline_t) { detail::submit_command(detail::command::sceneNoOutline()); }
void configure(Renderable&, infinite_accumulation_t) { detail::submit_command(detail::command::sceneInfiniteAccumulation()); }

void configure(Renderable&, dark_ui_t b) { detail::set_ui_darkmode(b.active); }
void configure(Renderable&, background_color const& b) { detail::submit_command(detail::command::sceneBackgroundColor(b.inner, b.outer)); }
void configure(Renderable&, ssao_power const& b) { detail::submit_command(detail::command::sceneSsaoPower(b.power)); }
void configure(Renderable&, ssao_radius const& b) { detail::submit_command(detail::command::sceneSsaoRadius(b.radius)); }
void configure(Renderable&, const tonemap_exposure_t& b) { detail::submit_command(detail::command::sceneTonemapping(b.exposure)); }
void configure(Renderable&, const camera_orientation& b)
{
    detail::submit_command(detail::command::sceneCameraOrientation(b.azimuth, b.altitude, b.distance));
}
void configure(Renderable&, const camera_transform& b) { detail::submit_command(detail::command::sceneCameraTransform(b.pos, b.target)); }

void configure(Renderable& r, const char* name) { r.name(name); }
void configure(Renderable& r, std::string_view name) { r.name(std::string(name)); }
void configure(Renderable& r, const std::string& name) { r.name(name); }

void configure(GeometricRenderable& r, transparent_t) { r.setRenderMode(GeometricRenderable::RenderMode::Transparent); }
void configure(GeometricRenderable& r, opaque_t) { r.setRenderMode(GeometricRenderable::RenderMode::Opaque); }
void configure(GeometricRenderable& r, no_fresnel_t) { r.setFresnel(false); }

void configure(GeometricRenderable& r, backface_culling_t b) { r.setBackfaceCullingEnabled(b.active); }

void configure(Renderable&, clear_accumulation_t b)
{
    if (b.active)
        detail::submit_command(detail::command::sceneClearAccum());
}

void configure(Renderable&, subview_margin_t b)
{
    detail::set_subview_margin(b.pixels);
    detail::set_subview_margin_color(b.color);
}

void configure(Renderable&, const headless_screenshot& b) { detail::set_headless_screenshot(b.resolution, b.accumulationCount, b.filename); }

void configure(Renderable&, SharedRenderable const& rj) { submit_command(detail::command::addRenderjob(rj)); }
}
