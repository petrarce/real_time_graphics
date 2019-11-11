#pragma once

#include <typed-geometry/detail/scalar_traits.hh>
#include <typed-geometry/types/mat.hh>
#include <typed-geometry/types/vec.hh>
#include "determinant.hh"

namespace tg
{
template <class ScalarT>
TG_NODISCARD constexpr mat<1, 1, fractional_result<ScalarT>> inverse(mat<1, 1, ScalarT> const& m)
{
    return tg::mat1::from_cols(ScalarT(1.0) / m[0].x);
}
template <class ScalarT>
TG_NODISCARD constexpr mat<2, 2, fractional_result<ScalarT>> inverse(mat<2, 2, ScalarT> const& m)
{
    auto invdet = ScalarT(1.0) / determinant(m);

    mat<2, 2, fractional_result<ScalarT>> res;
    res[0][0] = +m[1][1] * invdet;
    res[0][1] = -m[0][1] * invdet;
    res[1][0] = -m[1][0] * invdet;
    res[1][1] = +m[0][0] * invdet;
    return res;
}
template <class ScalarT>
TG_NODISCARD constexpr mat<3, 3, fractional_result<ScalarT>> inverse(mat<3, 3, ScalarT> const& m)
{
    auto invdet = ScalarT(1.0) / determinant(m);

    mat<3, 3, fractional_result<ScalarT>> res;
    res[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
    res[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * invdet;
    res[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * invdet;
    res[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * invdet;
    res[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * invdet;
    res[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * invdet;
    res[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
    res[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * invdet;
    res[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;
    return res;
}
template <class ScalarT>
TG_NODISCARD constexpr mat<4, 4, fractional_result<ScalarT>> inverse(mat<4, 4, ScalarT> const& m)
{
    auto invdet = ScalarT(1.0) / determinant(m);

    mat<4, 4, fractional_result<ScalarT>> res;

    res[0][0] = m[1][1] * m[2][2] * m[3][3] - m[1][1] * m[2][3] * m[3][2] - m[2][1] * m[1][2] * m[3][3] + m[2][1] * m[1][3] * m[3][2]
                + m[3][1] * m[1][2] * m[2][3] - m[3][1] * m[1][3] * m[2][2];
    res[1][0] = -m[1][0] * m[2][2] * m[3][3] + m[1][0] * m[2][3] * m[3][2] + m[2][0] * m[1][2] * m[3][3] - m[2][0] * m[1][3] * m[3][2]
                - m[3][0] * m[1][2] * m[2][3] + m[3][0] * m[1][3] * m[2][2];
    res[2][0] = m[1][0] * m[2][1] * m[3][3] - m[1][0] * m[2][3] * m[3][1] - m[2][0] * m[1][1] * m[3][3] + m[2][0] * m[1][3] * m[3][1]
                + m[3][0] * m[1][1] * m[2][3] - m[3][0] * m[1][3] * m[2][1];
    res[3][0] = -m[1][0] * m[2][1] * m[3][2] + m[1][0] * m[2][2] * m[3][1] + m[2][0] * m[1][1] * m[3][2] - m[2][0] * m[1][2] * m[3][1]
                - m[3][0] * m[1][1] * m[2][2] + m[3][0] * m[1][2] * m[2][1];
    res[0][1] = -m[0][1] * m[2][2] * m[3][3] + m[0][1] * m[2][3] * m[3][2] + m[2][1] * m[0][2] * m[3][3] - m[2][1] * m[0][3] * m[3][2]
                - m[3][1] * m[0][2] * m[2][3] + m[3][1] * m[0][3] * m[2][2];
    res[1][1] = m[0][0] * m[2][2] * m[3][3] - m[0][0] * m[2][3] * m[3][2] - m[2][0] * m[0][2] * m[3][3] + m[2][0] * m[0][3] * m[3][2]
                + m[3][0] * m[0][2] * m[2][3] - m[3][0] * m[0][3] * m[2][2];
    res[2][1] = -m[0][0] * m[2][1] * m[3][3] + m[0][0] * m[2][3] * m[3][1] + m[2][0] * m[0][1] * m[3][3] - m[2][0] * m[0][3] * m[3][1]
                - m[3][0] * m[0][1] * m[2][3] + m[3][0] * m[0][3] * m[2][1];
    res[3][1] = m[0][0] * m[2][1] * m[3][2] - m[0][0] * m[2][2] * m[3][1] - m[2][0] * m[0][1] * m[3][2] + m[2][0] * m[0][2] * m[3][1]
                + m[3][0] * m[0][1] * m[2][2] - m[3][0] * m[0][2] * m[2][1];
    res[0][2] = m[0][1] * m[1][2] * m[3][3] - m[0][1] * m[1][3] * m[3][2] - m[1][1] * m[0][2] * m[3][3] + m[1][1] * m[0][3] * m[3][2]
                + m[3][1] * m[0][2] * m[1][3] - m[3][1] * m[0][3] * m[1][2];
    res[1][2] = -m[0][0] * m[1][2] * m[3][3] + m[0][0] * m[1][3] * m[3][2] + m[1][0] * m[0][2] * m[3][3] - m[1][0] * m[0][3] * m[3][2]
                - m[3][0] * m[0][2] * m[1][3] + m[3][0] * m[0][3] * m[1][2];
    res[2][2] = m[0][0] * m[1][1] * m[3][3] - m[0][0] * m[1][3] * m[3][1] - m[1][0] * m[0][1] * m[3][3] + m[1][0] * m[0][3] * m[3][1]
                + m[3][0] * m[0][1] * m[1][3] - m[3][0] * m[0][3] * m[1][1];
    res[3][2] = -m[0][0] * m[1][1] * m[3][2] + m[0][0] * m[1][2] * m[3][1] + m[1][0] * m[0][1] * m[3][2] - m[1][0] * m[0][2] * m[3][1]
                - m[3][0] * m[0][1] * m[1][2] + m[3][0] * m[0][2] * m[1][1];
    res[0][3] = -m[0][1] * m[1][2] * m[2][3] + m[0][1] * m[1][3] * m[2][2] + m[1][1] * m[0][2] * m[2][3] - m[1][1] * m[0][3] * m[2][2]
                - m[2][1] * m[0][2] * m[1][3] + m[2][1] * m[0][3] * m[1][2];
    res[1][3] = m[0][0] * m[1][2] * m[2][3] - m[0][0] * m[1][3] * m[2][2] - m[1][0] * m[0][2] * m[2][3] + m[1][0] * m[0][3] * m[2][2]
                + m[2][0] * m[0][2] * m[1][3] - m[2][0] * m[0][3] * m[1][2];
    res[2][3] = -m[0][0] * m[1][1] * m[2][3] + m[0][0] * m[1][3] * m[2][1] + m[1][0] * m[0][1] * m[2][3] - m[1][0] * m[0][3] * m[2][1]
                - m[2][0] * m[0][1] * m[1][3] + m[2][0] * m[0][3] * m[1][1];
    res[3][3] = m[0][0] * m[1][1] * m[2][2] - m[0][0] * m[1][2] * m[2][1] - m[1][0] * m[0][1] * m[2][2] + m[1][0] * m[0][2] * m[2][1]
                + m[2][0] * m[0][1] * m[1][2] - m[2][0] * m[0][2] * m[1][1];

    return res * invdet;
}
} // namespace tg
