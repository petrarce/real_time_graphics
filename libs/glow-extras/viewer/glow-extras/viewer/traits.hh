#pragma once

#include <type_traits>

namespace glow
{
namespace viewer
{
namespace detail
{
struct type_like_helper
{
    template <class T>
    static auto is_pos2_like(T const& v) -> decltype(float(v[0]), float(v[1]), std::enable_if_t<sizeof(T) == 2 * sizeof(v[0]), std::true_type>());
    static std::false_type is_pos2_like(...);

    template <class T>
    static auto is_pos3_like(T const& v)
        -> decltype(float(v[0]), float(v[1]), float(v[2]), std::enable_if_t<sizeof(T) == 3 * sizeof(v[0]), std::true_type>());
    static std::false_type is_pos3_like(...);

    template <class T>
    static auto is_pos4_like(T const& v)
        -> decltype(float(v[0]), float(v[1]), float(v[2]), float(v[3]), std::enable_if_t<sizeof(T) == 4 * sizeof(v[0]), std::true_type>());
    static std::false_type is_pos4_like(...);

    template <class T>
    static auto is_color3_like(T const& v)
        -> decltype(float(v.r), float(v.g), float(v.b), std::enable_if_t<sizeof(T) == 3 * sizeof(v.r), std::true_type>());
    static std::false_type is_color3_like(...);

    template <class T>
    static auto is_color4_like(T const& v)
        -> decltype(float(v.r), float(v.g), float(v.b), float(v.a), std::enable_if_t<sizeof(T) == 4 * sizeof(v.r), std::true_type>());
    static std::false_type is_color4_like(...);
};

// ======================= Traits =======================
#define GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(trait) \
    template <class T>                          \
    constexpr bool trait = decltype(type_like_helper::trait(std::declval<T>()))::value // force ;

GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(is_pos2_like);
GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(is_pos3_like);
GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(is_pos4_like);

GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(is_color3_like);
GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT(is_color4_like);

#undef GLOW_VIEWER_IMPL_TYPE_LIKE_TRAIT
}
}
}
