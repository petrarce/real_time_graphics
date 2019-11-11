#pragma once

#include "../mat.hh"
#include "../pos.hh"
#include <typed-geometry/types/scalars/default.hh>
#include "../vec.hh"

#include "aabb.hh"

// An oriented box
// stored as:
//   center position
//   orthogonal matrix that maps the -1..1 cube to the oriented box (half-extents)

namespace tg
{
template <int D, class ScalarT>
struct box;

// Common box types

using box1 = box<1, f32>;
using box2 = box<2, f32>;
using box3 = box<3, f32>;
using box4 = box<4, f32>;

using fbox1 = box<1, f32>;
using fbox2 = box<2, f32>;
using fbox3 = box<3, f32>;
using fbox4 = box<4, f32>;

using dbox1 = box<1, f64>;
using dbox2 = box<2, f64>;
using dbox3 = box<3, f64>;
using dbox4 = box<4, f64>;

using ibox1 = box<1, i32>;
using ibox2 = box<2, i32>;
using ibox3 = box<3, i32>;
using ibox4 = box<4, i32>;

using ubox1 = box<1, u32>;
using ubox2 = box<2, u32>;
using ubox3 = box<3, u32>;
using ubox4 = box<4, u32>;


// ======== IMPLEMENTATION ========

template <int D, class ScalarT>
struct box
{
    using vec_t = vec<D, ScalarT>;
    using pos_t = pos<D, ScalarT>;
    using mat_t = mat<D, D, ScalarT>;

    static const box minus_one_to_one;
    static const box unit_from_zero;
    static const box unit_centered;

    pos_t center;
    mat_t half_extents;

    constexpr box() = default;
    constexpr box(pos_t center, mat_t const& half_extents) : center(center), half_extents(half_extents) {}
    constexpr box(aabb<D, ScalarT> const& b); // requires tg.hh

    TG_NODISCARD bool operator==(box const& rhs) const { return center == rhs.center && half_extents == rhs.half_extents; }
    TG_NODISCARD bool operator!=(box const& rhs) const { return !operator==(rhs); }
};
} // namespace tg
