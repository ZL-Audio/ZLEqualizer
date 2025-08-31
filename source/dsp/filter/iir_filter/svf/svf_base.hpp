// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include <vector>
#include <complex>
#include <span>

namespace zldsp::filter {
    template<typename FloatType>
    class SVFBase {
    public:
        SVFBase() = default;

        void prepare(const size_t num_channels) {
            s1_.resize(num_channels);
            s2_.resize(num_channels);
            reset();
        }

        void reset() {
            std::fill(s1_.begin(), s1_.end(), static_cast<FloatType>(0));
            std::fill(s2_.begin(), s2_.end(), static_cast<FloatType>(0));
        }

        template <bool isBypass = false>
        void process(std::span<FloatType*> buffer, const size_t num_samples) noexcept {
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                auto *samples = buffer[channel];
                for (size_t i = 0; i < num_samples; ++i) {
                    if constexpr (isBypass) {
                        processSample(channel, samples[i]);
                    } else {
                        samples[i] = processSample(channel, samples[i]);
                    }
                }
            }
        }

        FloatType processSample(const size_t channel, FloatType input) {
            const auto y_hp = h_ * (input - s1_[channel] * (g_ + R2_) - s2_[channel]);

            const auto y_bp = y_hp * g_ + s1_[channel];
            s1_[channel] = y_hp * g_ + y_bp;

            const auto y_lp = y_bp * g_ + s2_[channel];
            s2_[channel] = y_bp * g_ + y_lp;

            return chp_ * y_hp + cbp_ * y_bp + clp_ * y_lp;
        }

        void updateFromBiquad(const std::array<double, 6> &coeff) {
            const auto temp1 = std::sqrt(std::abs((-coeff[0] - coeff[1] - coeff[2])));
            const auto temp2 = std::sqrt(std::abs((-coeff[0] + coeff[1] - coeff[2])));
            g_ = static_cast<FloatType>(temp1 / temp2);
            R2_ = static_cast<FloatType>(2.0 * (coeff[0] - coeff[2]) / (temp1 * temp2));
            h_ = static_cast<FloatType>(1) / (g_ * (R2_ + g_) + static_cast<FloatType>(1));

            chp_ = static_cast<FloatType>((coeff[3] - coeff[4] + coeff[5]) / (coeff[0] - coeff[1] + coeff[2]));
            cbp_ = static_cast<FloatType>(2 * (coeff[5] - coeff[3]) / (temp1 * temp2));
            clp_ = static_cast<FloatType>((coeff[3] + coeff[4] + coeff[5]) / (coeff[0] + coeff[1] + coeff[2]));
        }

    private:
        FloatType g_, R2_, h_, chp_, cbp_, clp_;
        std::vector<FloatType> s1_, s2_;
    };
}
