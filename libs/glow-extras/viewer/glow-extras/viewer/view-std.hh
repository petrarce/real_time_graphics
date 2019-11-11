#pragma once

#include "view.hh"

namespace std
{
// inject view(...) into std
// makes the following work via ADL:
//   std::vector<glm::vec3> pts = ...;
//   view(pts);
//
// NOTE: this is technically undefined behavior!
//       (but works with our compilers)
//       include this header on your own risk!
using ::glow::viewer::view;
}
