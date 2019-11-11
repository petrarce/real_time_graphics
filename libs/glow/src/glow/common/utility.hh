#pragma once

namespace glow
{
/// e.g. for static_assert(always_false<T>, "this should not compile");
///      (if the static assert should only trigger on template instantiation)
template <class T>
inline constexpr bool always_false = false;
}
