// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef STATIC_GAIN_COMPENSATION_HPP
#define STATIC_GAIN_COMPENSATION_HPP

#include <juce_dsp/juce_dsp.h>
#include "empty_filter.hpp"

namespace zlFilter {
    template<typename FloatType>
    class StaticGainCompensation {
    public:
        explicit StaticGainCompensation(Empty<FloatType> &filter)
            : target(filter) {}

        FloatType getGain() {
            if (toUpdate.exchange(false)) {
                update();
            }
            return juce::Decibels::decibelsToGain(gainDB);
        }

        void setToUpdate() {
            toUpdate.store(true);
        }

    private:
        Empty<FloatType> &target;
        FloatType gainDB{0};
        std::atomic<bool> isON{false}, toUpdate{false};

        static inline FloatType integrateFQ(const FloatType f1, const FloatType f2) {
            const auto w1 = FloatType(1.0000057078597646) + FloatType(1.3450513160225395 * 1e-8) * f1 * f1;
            const auto w2 = FloatType(1.0000057078597646) + FloatType(1.3450513160225395 * 1e-8) * f2 * f2;
            return static_cast<FloatType>(std::log((w1 + 1) * (1 - w2) / (w2 + 1) / (1 - w1)));
        }

        static constexpr FloatType k1 = FloatType(0.165602);
        static constexpr FloatType k2 = FloatType(0.338973);
        static constexpr FloatType k3 = FloatType(0.712232);
        static constexpr FloatType k4 = FloatType(0.374335);
        static constexpr FloatType k5 = FloatType(1.494580);
        static constexpr FloatType k6 = FloatType(7.131157);
        static constexpr FloatType k7 = FloatType(0.014366);
        static constexpr std::array<FloatType, 3> pps{
            FloatType(0.6797385437634612),
            FloatType(0.6501623179337382),
            FloatType(0.1661043031674446),
        };
        static constexpr std::array<FloatType, 3> pns{
            FloatType(1.0005839027125558),
            FloatType(0.2615438074138483),
            FloatType(0.0876180361048472),
        };

        static constexpr std::array<FloatType, 3> lps{
            FloatType(0.5615303279130026),
            FloatType(1.0955796383939556),
            FloatType(0.0578375534446572),
        };
        static constexpr std::array<FloatType, 3> lns{
            FloatType(1.7666900390139590),
            FloatType(-0.9879875452397923),
            FloatType(0.0466874416227134),
        };

        static constexpr std::array<FloatType, 3> hps{
            FloatType(-1.6271905034386083),
            FloatType(2.6722453328537070),
            FloatType(0.1780141475194901),
        };
        static constexpr std::array<FloatType, 3> hns{
            FloatType(-0.0999799556355004),
            FloatType(1.0888973867418563),
            FloatType(0.0760070892708112),
        };

        static FloatType getPeakEstimation(const FloatType f, const FloatType g, const FloatType q) {
            const auto bw = static_cast<FloatType>(std::asinh(0.5 / q) / std::log(2));
            const auto scale = static_cast<FloatType>(std::pow(2, bw / 2));
            const auto f1 = juce::jlimit(FloatType(10), FloatType(20000), f / scale);
            const auto f2 = juce::jlimit(FloatType(10), FloatType(20000), f * scale);
            const auto fqEffect = integrateFQ(f1, f2);
            if (g > 0) {
                return -std::max(FloatType(0), getEstimation(fqEffect, bw, g, pps));
            } else {
                return -std::min(FloatType(0), getEstimation(fqEffect, bw, g, pns));
            }
        }

        static FloatType getLowShelfEstimation(FloatType f, const FloatType g) {
            f = juce::jlimit(FloatType(15), FloatType(5000), f);
            const auto bw = static_cast<FloatType>(std::log2(f / FloatType(10)));
            const auto fqEffect = integrateFQ(10, f);
            if (g > 0) {
                return -std::max(FloatType(0), getEstimation(fqEffect, bw, g, lps));
            } else {
                return -std::min(FloatType(0), getEstimation(fqEffect, bw, g, lns));
            }
        }

        static FloatType getHighShelfEstimation(FloatType f, const FloatType g) {
            f = juce::jlimit(FloatType(200), FloatType(18000), f);
            const auto bw = static_cast<FloatType>(std::log2(FloatType(20000) / f));
            const auto fqEffect = integrateFQ(f, 20000);
            if (g > 0) {
                return -std::max(FloatType(0), getEstimation(fqEffect, bw, g, hps));
            } else {
                return -std::min(FloatType(0), getEstimation(fqEffect, bw, g, hns));
            }
        }

        static FloatType getEstimation(FloatType fq_effect, FloatType bw, FloatType g,
                                       const std::array<FloatType, 3> &x) {
            return (x[0] * fq_effect + x[1] * bw) * g * x[2];
        }

        void update() {
            switch (target.getFilterType()) {
                case peak: {
                    const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                    const auto f = target.getFreq();
                    const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                    const auto q = juce::jlimit(FloatType(0.1), FloatType(5), target.getQ());
                    gainDB = (FloatType(1) + portion * FloatType(0.75)) * getPeakEstimation(f, g, q);
                    break;
                }
                case lowShelf: {
                    const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                    const auto f = target.getFreq();
                    const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                    gainDB = (FloatType(0.88) + portion * FloatType(0.66)) * getLowShelfEstimation(f, g);
                    break;
                }
                case highShelf: {
                    const auto portion = juce::jmax(std::abs(target.getGain()) - FloatType(12), FloatType(0)) / FloatType(18);
                    const auto f = target.getFreq();
                    const auto g = juce::jlimit(FloatType(-12), FloatType(12), target.getGain());
                    gainDB = (FloatType(0.5) + portion * FloatType(0.375)) * getHighShelfEstimation(f, g);
                    break;
                }
                case tiltShelf:
                case bandShelf:
                case lowPass:
                case highPass:
                case notch:
                case bandPass:
                default:
                    gainDB = FloatType(0);
                    break;
            }
        }
    };
} // zlIIR

#endif //STATIC_GAIN_COMPENSATION_HPP
