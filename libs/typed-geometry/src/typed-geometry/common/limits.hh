#pragma once

#include <typed-geometry/types/scalars/default.hh>

namespace tg
{
namespace detail
{
template <class T>
struct limits
{
    static constexpr T min(); ///< smallest finite element (NOTE: different from numeric_limits<T>::min)
    static constexpr T max(); ///< biggest finite element
};

template <>
struct limits<i8>
{
    static constexpr i8 min() { return i8(-128); }
    static constexpr i8 max() { return i8(127); }
};
template <>
struct limits<i16>
{
    static constexpr i16 min() { return i16(-32768); }
    static constexpr i16 max() { return i16(32767); }
};
template <>
struct limits<i32>
{
    static constexpr i32 min() { return i32(-2147483647 - 1); }
    static constexpr i32 max() { return i32(2147483647); }
};
#ifdef _MSC_VER
template <>
struct limits<long>
{
    static_assert(sizeof(long) == 4, "unsupported arch");
    static constexpr long min() { return long(-2147483647 - 1); }
    static constexpr long max() { return long(2147483647); }
};
#else
template <>
struct limits<long>
{
    static_assert(sizeof(long) == 8, "unsupported arch");
    static constexpr long min() { return long(-9223372036854775807LL - 1); }
    static constexpr long max() { return long(9223372036854775807LL); }
};
#endif
template <>
struct limits<long long>
{
    static constexpr long long min() { return i64(-9223372036854775807LL - 1); }
    static constexpr long long max() { return i64(9223372036854775807LL); }
};

template <>
struct limits<u8>
{
    static constexpr u8 min() { return u8(0); }
    static constexpr u8 max() { return u8(255); }
};
template <>
struct limits<u16>
{
    static constexpr u16 min() { return u16(0); }
    static constexpr u16 max() { return u16(65535); }
};
template <>
struct limits<u32>
{
    static constexpr u32 min() { return u32(0); }
    static constexpr u32 max() { return u32(4294967295); }
};
#ifdef _MSC_VER
template <>
struct limits<unsigned long>
{
    static_assert(sizeof(unsigned long) == 4, "unsupported arch");
    static constexpr unsigned long min() { return u32(0); }
    static constexpr unsigned long max() { return u32(4294967295); }
};
#else
template <>
struct limits<unsigned long>
{
    static_assert(sizeof(unsigned long) == 8, "unsupported arch");
    static constexpr unsigned long min() { return u64(0); }
    static constexpr unsigned long max() { return u64(18446744073709551615uLL); }
};
#endif
template <>
struct limits<unsigned long long>
{
    static constexpr unsigned long long min() { return u64(0); }
    static constexpr unsigned long long max() { return u64(18446744073709551615uLL); }
};

template <>
struct limits<f8>
{
    static constexpr f8 min() { return f8(0.0f); }
    static constexpr f8 max() { return f8(1.0f); }
};
template <>
struct limits<f16>
{
    // TODO
    // static constexpr f16 min() { return f16(-32768); }
    // static constexpr f16 max() { return f16(32767); }
};
template <>
struct limits<f32>
{
    static constexpr f32 min() { return f32(-3.402823466e+38F); }
    static constexpr f32 max() { return f32(+3.402823466e+38F); }
};
template <>
struct limits<f64>
{
    static constexpr f64 min() { return f64(-1.7976931348623158e+308); }
    static constexpr f64 max() { return f64(+1.7976931348623158e+308); }
};

} // namespace detail
} // namespace tg
