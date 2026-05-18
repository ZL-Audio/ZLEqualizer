// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../calculator_base.hpp"

namespace zldsp::filter {
    template <size_t kFilterNum, size_t kFilterSize>
    class MatchCalculator final : public CorrectionCalculator<kFilterNum, kFilterSize> {
    public:
        static constexpr size_t kStartDecayIdx = 2, kEndDecayIdx = 32;

        MatchCalculator() :
            CorrectionCalculator<kFilterNum, kFilterSize>() {
            decays_.resize(kEndDecayIdx);
            const auto k1 = std::log(static_cast<float>(kStartDecayIdx));
            const auto k2 = std::log(static_cast<float>(kEndDecayIdx));
            const auto k3 = 1.f / (k2 - k1);
            for (size_t i = 0; i < kStartDecayIdx; ++i) {
                decays_[i] = 0.f;
            }
            for (size_t i = kStartDecayIdx; i < kEndDecayIdx; ++i) {
                const auto t = std::clamp((std::log(static_cast<float>(i)) - k1) * k3, 0.f, 1.f);
                decays_[i] = t * t * (3.f - 2.f * t);
            }
        }

    private:
        std::vector<float> decays_{};

        void prepareCorrection(const size_t) override {
        }

        void updateCorrection(const size_t idx) override {
            auto& proto_real{CorrectionCalculator<kFilterNum, kFilterSize>::proto_real_};
            auto& proto_imag{CorrectionCalculator<kFilterNum, kFilterSize>::proto_imag_};
            auto& biquad_real{CorrectionCalculator<kFilterNum, kFilterSize>::biquad_real_};
            auto& biquad_imag{CorrectionCalculator<kFilterNum, kFilterSize>::biquad_imag_};
            auto& corr_real{CorrectionCalculator<kFilterNum, kFilterSize>::corrections_real_[idx]};
            auto& corr_imag{CorrectionCalculator<kFilterNum, kFilterSize>::corrections_imag_[idx]};
            constexpr auto min_mag_sq = CorrectionCalculator<kFilterNum, kFilterSize>::kMinMagSqr;
            // decay region
            for (size_t w_idx = kStartDecayIdx; w_idx < kEndDecayIdx; ++w_idx) {
                const auto b_r = biquad_real[w_idx];
                const auto b_i = biquad_imag[w_idx];
                const auto b_mag_sq = b_r * b_r + b_i * b_i;
                if (b_mag_sq > min_mag_sq) {
                    const auto p_r = proto_real[w_idx];
                    const auto p_i = proto_imag[w_idx];
                    const auto z_r = (p_r * b_r + p_i * b_i) / b_mag_sq;
                    const auto z_i = (p_i * b_r - p_r * b_i) / b_mag_sq;
                    const auto z_mag = std::sqrt(z_r * z_r + z_i * z_i);
                    const auto z_arg = std::atan2(z_i, z_r);
                    const auto k = decays_[w_idx];
                    const auto m = std::fma(k, z_mag - 1.0f, 1.0f);
                    const auto theta = k * z_arg;
                    const auto mult_r = m * std::cos(theta);
                    const auto mult_i = m * std::sin(theta);
                    const auto c_r = corr_real[w_idx];
                    const auto c_i = corr_imag[w_idx];
                    corr_real[w_idx] = c_r * mult_r - c_i * mult_i;
                    corr_imag[w_idx] = c_r * mult_i + c_i * mult_r;
                }
            }
            // full region
            for (size_t w_idx = kEndDecayIdx; w_idx < proto_real.size(); ++w_idx) {
                const auto b_r = biquad_real[w_idx];
                const auto b_i = biquad_imag[w_idx];
                const auto b_mag_sq = b_r * b_r + b_i * b_i;
                if (b_mag_sq > min_mag_sq) {
                    const auto p_r = proto_real[w_idx];
                    const auto p_i = proto_imag[w_idx];
                    const auto div_r = (p_r * b_r + p_i * b_i) / b_mag_sq;
                    const auto div_i = (p_i * b_r - p_r * b_i) / b_mag_sq;
                    const auto c_r = corr_real[w_idx];
                    const auto c_i = corr_imag[w_idx];
                    corr_real[w_idx] = c_r * div_r - c_i * div_i;
                    corr_imag[w_idx] = c_r * div_i + c_i * div_r;
                }
            }
            // if a single correction is larger than 40 dB, scale back to 40 dB
            for (size_t w_idx = kStartDecayIdx; w_idx < corr_real.size(); ++w_idx) {
                const auto c_r = corr_real[w_idx];
                const auto c_i = corr_imag[w_idx];
                if (std::max(std::abs(c_r), std::abs(c_i)) > 100.f) {
                    const auto c_mag = std::sqrt(c_r * c_r + c_i * c_i);
                    const auto scale = 100.f / c_mag;
                    corr_real[w_idx] *= scale;
                    corr_imag[w_idx] *= scale;
                }
            }
        }
    };
}
