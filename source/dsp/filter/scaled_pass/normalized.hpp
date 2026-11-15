// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

#include "coeff.hpp"

namespace zldsp::filter::scaled_pass {
    inline constexpr size_t kMaxStates = Coefficients::kMaxFirst + 2 * Coefficients::kMaxSecond;

    struct Rotation {
        double cosine{}, sine{};

        Rotation() = default;

        explicit Rotation(const double value) :
            cosine(value), sine(std::sqrt(std::max(0.0, (1.0 - value) * (1.0 + value)))) {
        }
    };

    namespace normalized_detail {
        // input-normal section with the original cascade response
        struct Section {
            size_t offset{};
            double a00{}, a01{}, a10{}, a11{}, b0{}, b1{}, c0{}, c1{}, direct{};
            double denominator1{}, denominator2{}, input_sine{};
        };

        using Values = std::array<double, kMaxStates>;

        /**
         * project one real pole and update the residual in place
         * @return C*x, where x = (I - pole*A)^-1 residual
         */
        inline double projectFirst(const double pole, Values& residual,
                                   const std::span<const Section> first, const std::span<const Section> second) {
            double input{};
            for (const auto& s : first) {
                const auto k = s.offset;
                const auto x = (residual[k] + pole * s.b0 * input) / (1.0 - pole * s.a00);
                residual[k] = (s.a00 - pole) * x + s.b0 * input;
                input = s.c0 * x + s.direct * input;
            }
            for (const auto& s : second) {
                const auto k = s.offset;
                const auto r0 = residual[k] + pole * s.b0 * input;
                const auto r1 = residual[k + 1] + pole * s.b1 * input;
                const auto m00 = 1.0 - pole * s.a00, m01 = -pole * s.a01;
                const auto m10 = -pole * s.a10, m11 = 1.0 - pole * s.a11;
                const auto inverse = 1.0 / (m00 * m11 - m01 * m10);
                const auto x0 = (m11 * r0 - m01 * r1) * inverse;
                const auto x1 = (m00 * r1 - m10 * r0) * inverse;
                residual[k] = (s.a00 - pole) * x0 + s.a01 * x1 + s.b0 * input;
                residual[k + 1] = s.a10 * x0 + (s.a11 - pole) * x1 + s.b1 * input;
                input = s.c0 * x0 + s.c1 * x1 + s.direct * input;
            }
            return input;
        }

        /**
         * project a pole pair using real 2-by-2 solves and update the residual in place
         * @return {C*v, C*A*v}, where v = (I - trace*A + determinant*A*A)^-1 residual
         */
        inline std::array<double, 2> projectPair(const double trace, const double determinant, Values& residual,
                                               const std::span<const Section> sections) {
            double input{}, a_input{};
            for (const auto& s : sections) {
                const auto k = s.offset;
                // Cayley-Hamilton: J*J = trace(J)*J - det(J)*I
                const auto alpha = std::fma(-determinant, s.denominator2, 1.0);
                const auto beta = std::fma(-determinant, s.denominator1, -trace);
                // (J - trace(J)*I)*B = {input_sine, 0}
                const auto feed = -beta * input - determinant * a_input;
                const auto r0 = residual[k] + s.b0 * feed - determinant * s.input_sine * input;
                const auto r1 = residual[k + 1] + s.b1 * feed;
                const auto m00 = std::fma(beta, s.a00, alpha), m01 = beta * s.a01;
                const auto m10 = beta * s.a10, m11 = std::fma(beta, s.a11, alpha);
                const auto inverse = 1.0 / (m00 * m11 - m01 * m10);
                const auto v0 = (m11 * r0 - m01 * r1) * inverse;
                const auto v1 = (m00 * r1 - m10 * r0) * inverse;
                const auto av0 = s.a00 * v0 + s.a01 * v1 + s.b0 * input;
                const auto av1 = s.a10 * v0 + s.a11 * v1 + s.b1 * input;
                const auto trace_difference = -s.denominator1 - trace;
                const auto determinant_difference = determinant - s.denominator2;
                residual[k] = trace_difference * av0 + determinant_difference * v0
                    + s.input_sine * input + s.b0 * a_input;
                residual[k + 1] = trace_difference * av1 + determinant_difference * v1 + s.b1 * a_input;
                input = s.c0 * v0 + s.c1 * v1 + s.direct * input;
                a_input = s.c0 * av0 + s.c1 * av1 + s.direct * a_input;
            }
            return {input, a_input};
        }
    }

    /**
     * a normalized filter bank preserving the scaled-pass magnitude and phase response
     */
    struct NormalizedCoefficients {
        size_t first_count{}, second_count{};
        std::array<Rotation, Coefficients::kMaxFirst> first{};
        std::array<std::array<Rotation, 2>, Coefficients::kMaxSecond> second{};
        std::array<double, kMaxStates + 1> taps{};

        template <bool svf>
        void update(const Coefficients& coefficients) {
            using namespace normalized_detail;
            std::array<Section, Coefficients::kMaxFirst + Coefficients::kMaxSecond> sections{};
            Values residual{};
            first_count = coefficients.first_count;
            second_count = coefficients.second_count;
            double direct = 1.0;

            for (size_t i = 0; i < first_count; ++i) {
                const auto& c = coefficients.first[i];
                const auto delta = svf ? c[1] : c[0];
                const auto b0 = svf ? c[0] * c[1] : c[1];
                const auto sum = svf ? c[1] * c[2] : c[2];
                auto& rotation = first[i];
                rotation.cosine = 1.0 - delta;
                rotation.sine = std::sqrt(std::max(0.0, delta * (2.0 - delta)));
                auto& section = sections[i];
                section.offset = i;
                section.a00 = rotation.cosine;
                section.b0 = rotation.sine;
                section.c0 = std::fma(-delta, b0, sum) / rotation.sine;
                section.direct = b0;
                residual[i] = section.b0 * direct;
                direct *= b0;
            }

            for (size_t i = 0; i < second_count; ++i) {
                const auto& c = coefficients.second[i];
                double a1, a2, b0, b1, b2;
                if constexpr (svf) {
                    const auto c2_sqr = c[2] * c[2];
                    a1 = c[1] * (1.0 - c2_sqr);
                    a2 = std::fma(c[1], 1.0 + c2_sqr, -1.0);
                    b0 = c[1] * c[0];
                    b1 = c[1] * std::fma(-c[2], c[3], c[4]);
                    b2 = c[1] * (std::fma(c[2], c[3], c[4]) - c[0]);
                } else {
                    a1 = c[0]; a2 = c[1]; b0 = c[2]; b1 = c[3]; b2 = c[4];
                }
                auto& rotations = second[i];
                rotations[0] = Rotation(-a1 / (1.0 + a2));
                rotations[1] = Rotation(-a2);
                const auto b = rotations[0].cosine, bs = rotations[0].sine;
                const auto h = rotations[1].cosine, hs = rotations[1].sine;
                auto& section = sections[first_count + i];
                section.offset = first_count + 2 * i;
                section.a00 = -b * h; section.a01 = bs;
                section.a10 = bs * h; section.a11 = b;
                section.b0 = -b * hs; section.b1 = bs * hs;
                const auto numerator2 = std::fma(-b0, a2, b2);
                section.c0 = numerator2 / hs;
                section.c1 = std::fma(b, numerator2, std::fma(-b0, a1, b1)) / (hs * bs);
                section.direct = b0;
                section.denominator1 = a1;
                section.denominator2 = a2;
                section.input_sine = hs;
                residual[section.offset] = section.b0 * direct;
                residual[section.offset + 1] = section.b1 * direct;
                direct *= b0;
            }
            taps[0] = direct;

            const auto pairs = std::span<const Section>(sections.data() + first_count, second_count);
            for (size_t i = 0; i < first_count; ++i) {
                const auto suffix = std::span<const Section>(sections.data() + i, first_count - i);
                taps[i + 1] = first[i].sine * projectFirst(first[i].cosine, residual, suffix, pairs);
            }
            for (size_t i = 0; i < second_count; ++i) {
                const auto& section = pairs[i];
                const auto [cv, cav] = projectPair(-section.denominator1, section.denominator2,
                                                 residual, pairs.subspan(i));
                const auto& rotations = second[i];
                const auto b = rotations[0].cosine, bs = rotations[0].sine;
                const auto hs = rotations[1].sine;
                taps[section.offset + 1] = hs * std::fma(-b, cv, cav);
                taps[section.offset + 2] = hs * bs * cv;
            }
        }
    };
}
