#pragma once

#include <typed-geometry/tg-lean.hh>

namespace glow::viewer
{
struct RenderInfo
{
    tg::mat4 const& view;
    tg::mat4 const& proj;
    tg::pos3 const& sunPos;
    tg::isize2 resolution;
    tg::pos3 camPos;
    int accumulationCount;
};
}
