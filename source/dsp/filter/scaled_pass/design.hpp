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
#include <functional>
#include "coeff.hpp"

namespace zldsp::filter::scaled_pass {
    struct Coverage {
        double min_cutoff{10.0}, max_cutoff{160000.0};
        double min_frequency{10.0}, max_frequency{};
        double tail_error{0.005}, spacing{2.0};
    };

    namespace detail {
        inline constexpr size_t kMaxFactors = Coefficients::kMaxOrder + Coefficients::kMaxTail;
        inline constexpr double kLn2 = std::numbers::ln2, kLn10 = std::numbers::ln10;

        struct Peak {
            double gain{};
            double q{};
        };

        struct Knee {
            bool single{};
            std::size_t panels{};
            double gain_scale{};
            double q{};
            double q_step{};
            double spread{};
        };

        struct Plan {
            std::size_t order{};
            std::size_t tail_count{};
            std::size_t native_count{};
            std::size_t knee_count{};
            std::size_t family_count{};
            double step{};
            double last_root{};
            std::array<double, kMaxFactors> nodes{};
            std::array<double, kMaxFactors> roots{};
            std::array<Knee, Coefficients::kMaxOrder / 2> families{};
            bool ready{};

            /**
             * prepare the fixed section layout
             * @param new_order 1 or an even order in [2, Coefficients::kMaxOrder]
             * @param new_tail_count the tail section count in [1, Coefficients::kMaxTail]
             * @param spacing finite octave spacing in (0, 2.5 / ln(2)]
             */
            void prepare(const std::size_t new_order, const std::size_t new_tail_count,
                         const double spacing = 1.0) noexcept {
                order = new_order;
                tail_count = new_tail_count;
                step = spacing * kLn2;
                native_count = order == 1 ? 1 : order / 2;
                knee_count = 0;
                family_count = 0;
                for (std::size_t k = 0; k < order + tail_count; ++k) {
                    roots[k] = std::exp(-step * (static_cast<double>(k) + 0.5));
                    nodes[k] = roots[k] * roots[k];
                }
                last_root = roots[tail_count - 1];

                if (order > 1) {
                    for (std::size_t i = 0; i < order / 2; ++i) {
                        const auto theta = pi * static_cast<double>(2 * i + 1) /
                            (2.0 * static_cast<double>(order));
                        const auto v = -std::log(std::cos(theta)); // ln(2*Q_i^BW)
                        auto& family = families[family_count++];
                        if (v / kLn2 <= 1.0) {
                            family.single = true;
                            family.panels = 1;
                            family.gain_scale = -20.0 * v / kLn10;
                            family.q = 0.5 * std::exp(v * 0.5);
                            family.q_step = 1.0;
                            family.spread = 0.0;
                            ++knee_count;
                        } else {
                            family.single = false;
                            family.panels = static_cast<std::size_t>(std::ceil((v / kLn2) / 2.5));
                            const auto width = v / static_cast<double>(family.panels);
                            family.gain_scale = -10.0 * width / kLn10;
                            family.q = 0.5 * std::exp(width * 0.5);
                            family.q_step = std::exp(width);
                            family.spread = width / (4.0 * std::sqrt(3.0));
                            knee_count += 2 * family.panels;
                        }
                    }
                }
                ready = true;
            }
        };

        struct ScaleCache {
            double scale{1.0};
            std::array<double, kMaxFactors> nodes{};
            std::array<double, kMaxFactors> roots{};
            std::array<Peak, Coefficients::kMaxKnee> knee{};

            /**
             * update the scale-dependent nodes and knee sections
             * @param plan a prepared section layout
             * @param new_scale a finite magnitude scale in [0, 1]
             */
            void update(const Plan& plan, const double new_scale) noexcept {
                scale = new_scale;
                const auto order = plan.order;
                const auto tail_count = plan.tail_count;
                const auto b = 1.0 - scale;
                const auto beta = static_cast<double>(order) * b;
                const auto shift = std::exp(plan.step * beta);
                for (std::size_t k = 0; k < order + tail_count; ++k) {
                    const auto t = static_cast<double>(k) + 1.0 - beta;
                    double root{};
                    if (t <= 0.0) {
                        root = 1.0;
                    } else if (t < 1.0) {
                        root = std::exp(-0.5 * plan.step * t * t);
                    } else if (t <= static_cast<double>(tail_count)) {
                        root = plan.roots[k] * shift;
                    } else if (t < static_cast<double>(tail_count) + 1.0) {
                        const auto s = static_cast<double>(tail_count) + 1.0 - t;
                        const auto polynomial = (5.0 - 2.0 * plan.step) +
                            (2.0 * plan.step - 4.0) * s;
                        root = plan.last_root * s * s * std::sqrt(polynomial);
                    }
                    roots[k] = root;
                    nodes[k] = root * root;
                }

                const auto shape = std::sqrt(4.0 - b * b);
                std::size_t peak_index{};
                for (std::size_t i = 0; i < plan.family_count; ++i) {
                    const auto& family = plan.families[i];
                    const auto gain = family.gain_scale * b;
                    if (family.single) {
                        knee[peak_index++] = {gain, family.q};
                    } else {
                        const auto spread = std::exp(family.spread * shape);
                        auto q = family.q;
                        for (std::size_t j = 0; j < family.panels; ++j) {
                            knee[peak_index++] = {gain, q / spread};
                            knee[peak_index++] = {gain, q * spread};
                            q *= family.q_step;
                        }
                    }
                }
            }
        };

        /**
         * calculate the tail section count, with a minimum of one
         * @param sample_rate a finite, positive sample rate in Hz
         * @param hp whether the filter is a high-pass filter
         * @param coverage valid response coverage requiring at most Coefficients::kMaxTail sections
         */
        inline size_t getTailCount(const double sample_rate, const bool hp, const Coverage& coverage) {
            const auto high = coverage.max_frequency > 0.0 ? coverage.max_frequency : sample_rate * 0.5;
            const auto f = hp ? coverage.min_frequency : high;
            const auto cutoff = hp ? coverage.max_cutoff : coverage.min_cutoff;
            const auto w = ivantsov::getWrappingW2(cutoff / sample_rate);
            const auto cot = 1.0 / std::tan(pi * f / sample_rate);
            const auto t = ivantsov::kSigmaSq + pi * pi * cot * cot;
            const auto digital_y = std::sqrt(hp ? t / w : w / t);
            const auto analog_y = hp ? cutoff / f : f / cutoff;
            const auto tolerance = std::expm1(coverage.tail_error * kLn10 / 10.0);
            const auto count = std::ceil(0.5 + std::log2(std::max(digital_y, analog_y)
                / std::sqrt(tolerance)) / coverage.spacing);
            return static_cast<size_t>(std::max(1.0, count));
        }
    }

    template <class Coeff>
    class Designer {
    public:
        /**
         * set the cutoff and response ranges used to size the tail
         * all values must be finite, with 0 < min_cutoff <= max_cutoff and
         * 0 < min_frequency < max_frequency <= sample_rate / 2; max_frequency = 0 uses Nyquist.
         * tail_error must be positive and spacing must be in (0, 2.5 / ln(2)].
         * the resulting tail count must not exceed Coefficients::kMaxTail.
         * @param coverage the ranges, tail error in dB and octave spacing
         */
        void setCoverage(const Coverage& coverage) {
            coverage_ = coverage;
            plan_.ready = false;
        }

        /**
         * update the scaled-pass coefficients
         * all floating-point parameters must be finite.
         * @param type kScaledLowPass or kScaledHighPass
         * @param order 1 or an even order in [2, Coefficients::kMaxOrder]
         * @param frequency the cutoff in Hz, within the configured cutoff range
         * @param sample_rate a positive sample rate in Hz supporting the configured response coverage
         * @param q a positive Q value
         * @param scale the magnitude scale in [0, 1]
         */
        void update(const FilterType type, const size_t order, const double frequency,
                    const double sample_rate, const double q, const double scale) {
            const auto hp = isHighPass(type);
            const auto same = std::equal_to<double>{};
            const auto plan_changed = !plan_.ready || order != plan_.order ||
                !same(sample_rate, sample_rate_) || type != type_;
            if (plan_changed) {
                const auto count = detail::getTailCount(sample_rate, hp, coverage_);
                plan_.prepare(order, count, coverage_.spacing);
                updateGroups();
            }
            const auto frequency_changed = plan_changed || !same(frequency, frequency_);
            const auto scale_changed = plan_changed || !same(scale, scale_.scale);
            if (scale_changed) { scale_.update(plan_, scale); }
            if (frequency_changed) {
                context_ = Backend<Coeff>::getContext(frequency / sample_rate);
                for (size_t i = 0; i < plan_.tail_count; ++i) {
                    denominators_[i] = Backend<Coeff>::getFactor(context_, hp, plan_.nodes[i], plan_.roots[i]);
                }
            }
            if (frequency_changed || scale_changed) {
                for (size_t i = 0; i < order + plan_.tail_count; ++i) {
                    numerators_[i] = Backend<Coeff>::getFactor(context_, hp, scale_.nodes[i], scale_.roots[i]);
                }
                for (size_t i = 0; i < plan_.tail_count; ++i) {
                    coefficients_.first[i] = Backend<Coeff>::getPassRatio(
                        context_, numerators_[order + i], denominators_[i]);
                }
                for (size_t i = 0; i < plan_.knee_count; ++i) {
                    coefficients_.second[i] = Backend<Coeff>::getPassPeak(
                        context_, scale_.knee[i].gain, scale_.knee[i].q);
                }
            }
            const auto native_changed = frequency_changed || !same(q, q_);
            if (native_changed) {
                if (order == 1) {
                    native_first_ = Backend<Coeff>::get1Pass(context_, hp);
                } else {
                    const PassQDistribution distribution(order, q);
                    for (size_t i = 0; i < plan_.native_count; ++i) {
                        native_second_[i] = Backend<Coeff>::get2Pass(context_, hp, distribution.get(i));
                    }
                }
            }
            if (native_changed || scale_changed) {
                if (order == 1) {
                    coefficients_.first[plan_.tail_count] = Backend<Coeff>::get1ModifiedPass(
                        context_, native_first_, numerators_[0]);
                } else {
                    for (size_t i = 0; i < plan_.native_count; ++i) {
                        coefficients_.second[plan_.knee_count + i] = Backend<Coeff>::get2ModifiedPass(
                            context_, native_second_[i], numerators_[2 * i], numerators_[2 * i + 1]);
                    }
                }
            }
            frequency_ = frequency;
            sample_rate_ = sample_rate;
            q_ = q;
            type_ = type;
        }

        const Coefficients& getCoefficients() const { return coefficients_; }

    private:
        Coverage coverage_{};
        detail::Plan plan_{};
        detail::ScaleCache scale_{};
        Coefficients coefficients_{};
        Context context_{};
        std::array<Factor, detail::kMaxFactors> numerators_{};
        std::array<Factor, Coefficients::kMaxTail> denominators_{};
        std::array<double, 3> native_first_{};
        std::array<std::array<double, 5>, Coefficients::kMaxOrder / 2> native_second_{};
        double frequency_{}, sample_rate_{}, q_{};
        FilterType type_{kScaledLowPass};

        void updateGroups() {
            coefficients_.first_count = plan_.tail_count + (plan_.order == 1);
            coefficients_.second_count = plan_.order / 2 + plan_.knee_count;
            coefficients_.group_count = plan_.native_count;
            coefficients_.first_groups.fill(0);
            coefficients_.second_groups.fill(0);
            for (size_t i = 0; i < plan_.tail_count; ++i) {
                coefficients_.first_groups[i] = plan_.order == 1 ? 0 : (i % plan_.order) / 2;
            }
            size_t peak{};
            for (size_t i = 0; i < plan_.family_count; ++i) {
                const auto count = plan_.families[i].single ? 1 : 2 * plan_.families[i].panels;
                for (size_t j = 0; j < count; ++j) { coefficients_.second_groups[peak++] = i; }
            }
            if (plan_.order > 1) {
                for (size_t i = 0; i < plan_.native_count; ++i) {
                    coefficients_.second_groups[plan_.knee_count + i] = i;
                }
            }
        }
    };
}
