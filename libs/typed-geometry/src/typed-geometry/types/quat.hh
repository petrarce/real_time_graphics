#pragma once

#include <typed-geometry/types/scalars/default.hh>
#include "../detail/comp_traits.hh"
#include "../detail/macros.hh"
#include "../detail/scalar_traits.hh"
#include "../detail/utility.hh"
#include "fwd.hh"
#include "vec.hh"

namespace tg
{
template <class ScalarT>
struct quaternion;

using quat = quaternion<float>;
using fquat = quaternion<float>;
using dquat = quaternion<double>;

template <class ScalarT>
struct quaternion
{
    ScalarT x = ScalarT(0); // imag x
    ScalarT y = ScalarT(0); // imag y
    ScalarT z = ScalarT(0); // imag z
    ScalarT w = ScalarT(0); // real

    static const quaternion zero;
    static const quaternion identity;

    TG_DECLARE_COMP_TYPE_4_NO_SWIZZLE(quaternion);

    constexpr quaternion(vec<3, ScalarT> const& i, ScalarT r) : x(i.x), y(i.y), z(i.z), w(r) {}

    TG_NODISCARD constexpr bool operator==(quaternion const& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    TG_NODISCARD constexpr bool operator!=(quaternion const& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

    TG_NODISCARD static constexpr quaternion from_axis_angle(dir<3, ScalarT> const& axis, angle_t<ScalarT> angle);
    TG_NODISCARD static constexpr quaternion from_rotation_matrix(tg::mat<3, 3, ScalarT> const& m);
    TG_NODISCARD static constexpr quaternion from_rotation_matrix(tg::mat<4, 4, ScalarT> const& m);

    TG_NODISCARD constexpr explicit operator mat<3, 3, ScalarT>() const;
    TG_NODISCARD constexpr explicit operator mat<4, 4, ScalarT>() const;
};
template <class T>
const quaternion<T> quaternion<T>::zero = {T(0), T(0), T(0), T(0)};
template <class T>
const quaternion<T> quaternion<T>::identity = {T(0), T(0), T(0), T(1)};
}
