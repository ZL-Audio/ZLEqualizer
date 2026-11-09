// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <complex>
#include <limits>
#include "../helpers.hpp"
#include "../iir_filter/coeff/ivantsov_common.hpp"
#include "../iir_filter/coeff/ivantsov_coeff.hpp"
#include "../iir_filter/coeff/ivantsov_svf_coeff.hpp"
#include "../ideal_filter/coeff/ideal_coeff.hpp"

namespace zldsp::filter::scaled_pass {
    struct Context {
        double w{}, root{};
    };

    struct Factor {
        double b0{}, b1{}, sum{}, difference{};
    };

    struct Coefficients {
        static constexpr size_t kMaxOrder = 16, kMaxTail = 64, kMaxKnee = 13;
        static constexpr size_t kMaxFirst = kMaxTail + 1;
        static constexpr size_t kMaxSecond = kMaxOrder / 2 + kMaxKnee;

        size_t first_count{}, second_count{}, group_count{};
        std::array<size_t, kMaxFirst> first_groups{};
        std::array<size_t, kMaxSecond> second_groups{};
        std::array<std::array<double, 3>, kMaxFirst> first{};
        std::array<std::array<double, 5>, kMaxSecond> second{};

        [[nodiscard]] size_t size() const {
            return first_count + second_count;
        }
    };

    namespace detail {
        template <class Coeff>
        struct DigitalBackend {
            static Context getContext(const double w0) {
                const auto w = ivantsov::getWrappingW2(w0);
                return {w, std::sqrt(w)};
            }

            static Factor getFactor(const Context context, const bool hp, const double tau, const double sqrt_tau) {
                if (!hp) {
                    const auto r = std::sqrt(ivantsov::kSigmaSq + context.w * tau);
                    const auto inv = 1.0 / (pi + ivantsov::kSigma);
                    return {(pi + r) * inv, (pi - r) * inv, 2.0 * pi * inv, 2.0 * r * inv};
                }
                const auto v = sqrt_tau / context.root;
                const auto r = std::sqrt(1.0 + ivantsov::kSigmaSq * v * v);
                const auto t = pi * v;
                return {r + t, t - r, 2.0 * t, 2.0 * r};
            }

            static std::array<double, 3> get1Pass(const Context context, const bool hp) {
                return Coeff::get1PassFromW2(context.w, hp);
            }

            static std::array<double, 5> get2Pass(const Context context, const bool hp, const double q) {
                return Coeff::get2PassFromW2(context.w, hp, q);
            }

            static std::array<double, 5> getPassPeak(const Context context, const double gain, const double q) {
                return Coeff::getPeakFromW2(context.w, gain, q);
            }

            static std::complex<double> coordinate(const double w) { return std::polar(1.0, -w); }
        };
    }

    template <class Coeff>
    struct Backend;

    template <>
    struct Backend<IvantsovCoeff> : detail::DigitalBackend<IvantsovCoeff> {
        static std::array<double, 3> get1ModifiedPass(const Context context,
                                                      const std::array<double, 3>& native, const Factor factor) {
            const auto increment = 2.0 * pi / (pi + std::sqrt(context.w + ivantsov::kSigmaSq));
            return {increment, native[1] * factor.b0, native[1] * factor.sum};
        }

        static std::array<double, 5> get2ModifiedPass(const Context, const std::array<double, 5>& native,
                                                      const Factor left, const Factor right) {
            const auto b0 = native[2];
            return {native[0], native[1], b0 * left.b0 * right.b0,
                    b0 * (left.b0 * right.b1 + left.b1 * right.b0), b0 * left.b1 * right.b1};
        }

        static std::array<double, 3> getPassRatio(const Context,
                                                  const Factor numerator, const Factor denominator) {
            const auto inv = 1.0 / denominator.b0;
            return {denominator.sum * inv, numerator.b0 * inv, numerator.sum * inv};
        }

        static std::complex<double> response(const std::array<double, 3>& c, const std::complex<double> z) {
            const auto minus = 1.0 - z;
            return (c[1] * minus + c[2] * z) / (minus + c[0] * z);
        }

        static std::complex<double> response(const std::array<double, 5>& c, const std::complex<double> z) {
            return (c[2] + z * (c[3] + z * c[4])) / (1.0 + z * (c[0] + z * c[1]));
        }
    };

    template <>
    struct Backend<IvantsovSVFCoeff> : detail::DigitalBackend<IvantsovSVFCoeff> {
        static std::array<double, 3> get1ModifiedPass(const Context, const std::array<double, 3>& native,
                                                      const Factor& factor) {
            return {native[0] * factor.b0, native[1], native[0] * factor.sum};
        }

        static std::array<double, 5> get2ModifiedPass(const Context, const std::array<double, 5>& native,
                                                      const Factor& left, const Factor& right) {
            const auto c0 = native[0];
            return {c0 * left.b0 * right.b0, native[1], native[2],
                    c0 * left.difference * right.difference / (2.0 * native[2]),
                    c0 * 0.5 * left.sum * right.sum};
        }

        static std::array<double, 3> getPassRatio(const Context,
                                                  const Factor& numerator, const Factor& denominator) {
            return {numerator.b0 / denominator.sum, denominator.sum / denominator.b0,
                    numerator.sum / denominator.sum};
        }

        static std::complex<double> response(const std::array<double, 3>& c, const std::complex<double> z) {
            return (c[0] * c[1] * (1.0 - z) + c[1] * c[2] * z) / (1.0 - z + c[1] * z);
        }

        static std::complex<double> response(const std::array<double, 5>& c, const std::complex<double> z) {
            const auto minus = 1.0 - z, plus = 1.0 + z;
            return c[1] * (plus * (c[0] * minus + c[4] * z) - c[2] * c[3] * z * minus)
                / (minus * plus + c[1] * z * (plus - c[2] * c[2] * minus));
        }
    };

    template <>
    struct Backend<IdealCoeff> {
        static Context getContext(const double w0) {
            return {w0 * ppi, w0 * ppi};
        }

        static Factor getFactor(const Context, const bool hp,
                                const double, const double sqrt_tau) {
            return hp
                ? Factor{sqrt_tau, 1.0, 0.0, 0.0}
                : Factor{1.0, sqrt_tau, 0.0, 0.0};
        }

        static std::array<double, 3> get1Pass(const Context context, const bool hp) {
            return {context.w, hp ? 1.0 : 0.0, hp ? 0.0 : context.w};
        }

        static std::array<double, 5> get2Pass(const Context context, const bool hp, const double q) {
            const auto w2 = context.w * context.w;
            return {context.w / q, w2, hp ? 1.0 : 0.0, 0.0, hp ? 0.0 : w2};
        }

        static std::array<double, 5> getPassPeak(const Context context, const double g, const double q) {
            const auto w2 = context.w * context.w;
            const auto a = std::exp2(g * kDbToExp2Sqrt);
            return {context.w / a / q, w2, 1.0, context.w * a / q, w2};
        }

        static std::array<double, 3> get1ModifiedPass(const Context context, const std::array<double, 3>& native,
                                                      const Factor& factor) {
            return {native[0], factor.b1, context.w * factor.b0};
        }

        static std::array<double, 5> get2ModifiedPass(const Context context, const std::array<double, 5>& native,
                                                      const Factor& left, const Factor& right) {
            return {native[0], native[1], left.b1 * right.b1,
                    context.w * (left.b0 * right.b1 + left.b1 * right.b0),
                    context.w * context.w * left.b0 * right.b0};
        }

        static std::array<double, 3> getPassRatio(const Context context,
                                                  const Factor& numerator, const Factor& denominator) {
            return {context.w * denominator.b0 / denominator.b1, numerator.b1 / denominator.b1,
                    context.w * numerator.b0 / denominator.b1};
        }

        static std::complex<double> coordinate(const double w) { return {0.0, w}; }

        static std::complex<double> response(const std::array<double, 3>& c, const std::complex<double> s) {
            return (c[1] * s + c[2]) / (s + c[0]);
        }

        static std::complex<double> response(const std::array<double, 5>& c, const std::complex<double> s) {
            return (c[2] * s * s + c[3] * s + c[4]) / (s * s + c[0] * s + c[1]);
        }
    };

    inline double getIdealMagnitudeSquare(const Coefficients& coefficients, const double w) {
        const auto w2 = w * w;
        double result = 1.0;
        for (size_t i = 0; i < coefficients.first_count; ++i) {
            const auto& c = coefficients.first[i];
            result *= (c[1] * c[1] * w2 + c[2] * c[2]) / (w2 + c[0] * c[0]);
        }
        for (size_t i = 0; i < coefficients.second_count; ++i) {
            const auto& c = coefficients.second[i];
            const auto nr = c[4] - c[2] * w2, dr = c[1] - w2;
            result *= (nr * nr + c[3] * c[3] * w2) / (dr * dr + c[0] * c[0] * w2);
        }
        return result;
    }

    template <class Coeff>
    std::complex<double> getResponse(const Coefficients& coefficients,
                                     const std::complex<double> coordinate,
                                     const size_t group = std::numeric_limits<size_t>::max()) {
        std::complex<double> result{1.0, 0.0};
        for (size_t i = 0; i < coefficients.first_count; ++i) {
            if (group == std::numeric_limits<size_t>::max() || coefficients.first_groups[i] == group) {
                result *= Backend<Coeff>::response(coefficients.first[i], coordinate);
            }
        }
        for (size_t i = 0; i < coefficients.second_count; ++i) {
            if (group == std::numeric_limits<size_t>::max() || coefficients.second_groups[i] == group) {
                result *= Backend<Coeff>::response(coefficients.second[i], coordinate);
            }
        }
        return result;
    }

    template <class Coeff>
    std::complex<double> getResponse(const Coefficients& coefficients,
                                     const double w, const size_t group = std::numeric_limits<size_t>::max()) {
        return getResponse<Coeff>(coefficients, Backend<Coeff>::coordinate(w), group);
    }
}
