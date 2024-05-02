// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef STATIC_GAIN_COMPENSATION_HPP
#define STATIC_GAIN_COMPENSATION_HPP

#include "single_filter.hpp"

namespace zlIIR {
    template<typename FloatType>
    class StaticGainCompensation {
    public:
        explicit StaticGainCompensation(Filter<FloatType> &filter);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void update();

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void enable(const bool f) {
            isON.store(f);
            if (f) toUpdate.store(true);
        }

    private:
        Filter<FloatType> &target;
        juce::dsp::Gain<FloatType> gain;
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
        static constexpr std::array<FloatType, 5> pps{
            FloatType(0.0605857500413059),
            FloatType(0.0595128381706827),
            FloatType(0.3735951009224384),
            FloatType(0.5425036087957785),
            FloatType(-0.0336383285891464),
        };
        static constexpr std::array<FloatType, 5> pns{
            FloatType(0.0770122145524605),
            FloatType(0.0212413743693820),
            FloatType(-0.1887677967939364),
            FloatType(0.4059777723530943),
            FloatType(0.0800498794823235),
        };

        static constexpr std::array<FloatType, 5> lps{
            FloatType(0.0682443555510180),
            FloatType(0.0057106970019589),
            FloatType(-0.0161585411074138),
            FloatType(1.3024416303778590),
            FloatType(-1.1765258767845355),
        };
        static constexpr std::array<FloatType, 5> lns{
            FloatType(0.0825428757193346),
            FloatType(-0.0326573926413902),
            FloatType(0.0190833263448853),
            FloatType(1.1310539900637915),
            FloatType(-0.8724556311482653),
        };

        static constexpr std::array<FloatType, 5> hps{
            FloatType(-0.2142211518644782),
            FloatType(0.3723364793714626),
            FloatType(-0.0023831323818586),
            FloatType(3.9933845620563613),
            FloatType(-5.5364406039418395),
        };
        static constexpr std::array<FloatType, 5> hns{
            FloatType(-0.0613317836504331),
            FloatType(0.1869945607080542),
            FloatType(1.0128946247252020),
            FloatType(15.2852741225328312),
            FloatType(0.0212001385282977),
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
            f = juce::jlimit(FloatType(200), FloatType(16000), f);
            const auto bw = static_cast<FloatType>(std::log2(FloatType(20000) / f));
            const auto fqEffect = integrateFQ(f, 20000);
            if (g > 0) {
                return -std::max(FloatType(0), getEstimation(fqEffect, bw, g, hps));
            } else {
                return -std::min(FloatType(0), getEstimation(fqEffect, bw, g, hns));
            }
        }

        static FloatType getEstimation(FloatType fq_effect, FloatType bw, FloatType g,
                                       const std::array<FloatType, 5> &x) {
            return (x[0] * fq_effect + x[1] * bw) * g * (x[2] / (bw + x[3]) * g * (x[4] * g + 1) + 1);
        }
    };
} // zlIIR

#endif //STATIC_GAIN_COMPENSATION_HPP
