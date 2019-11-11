#pragma once

#include <typed-geometry/detail/macros.hh>
#include <typed-geometry/detail/scalar_traits.hh>
#include <typed-geometry/detail/sum_of_pos.hh>
#include <typed-geometry/types/dir.hh>
#include <typed-geometry/types/pos.hh>
#include <typed-geometry/types/size.hh>
#include <typed-geometry/types/vec.hh>

namespace tg
{
// pos +- vec = pos
TG_IMPL_DEFINE_BINARY_OP(pos, vec, pos, +);
TG_IMPL_DEFINE_BINARY_OP(pos, vec, pos, -);

// pos +- dir = pos
// TODO: is this required?
// TG_IMPL_DEFINE_BINARY_OP(pos, dir, pos, +);
// TG_IMPL_DEFINE_BINARY_OP(pos, dir, pos, -);

// pos */ size = pos
TG_IMPL_DEFINE_BINARY_OP(pos, size, pos, *);
TG_IMPL_DEFINE_BINARY_OP(pos, size, pos, /);

// pos - pos = vec
TG_IMPL_DEFINE_BINARY_OP(pos, pos, vec, -);

// +pos, -pos
TG_IMPL_DEFINE_UNARY_OP(pos, +);
TG_IMPL_DEFINE_UNARY_OP(pos, -);

// scalar OP pos, pos OP scalar
TG_IMPL_DEFINE_BINARY_OP_SCALAR(pos, -);
TG_IMPL_DEFINE_BINARY_OP_SCALAR(pos, +);
TG_IMPL_DEFINE_BINARY_OP_SCALAR(pos, *);
TG_IMPL_DEFINE_BINARY_OP_SCALAR_DIV(pos);

// special pos+pos handling
template <int D, class ScalarT>
TG_NODISCARD constexpr sum_of_pos<D, ScalarT> operator+(pos<D, ScalarT> const& a, pos<D, ScalarT> const& b)
{
    return sum_of_pos(a) + b;
}

// deprecated / not supported operations

} // namespace tg
