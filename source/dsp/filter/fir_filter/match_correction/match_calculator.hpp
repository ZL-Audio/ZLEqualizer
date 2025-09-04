// Copyright (C) 2025 - zsliu98
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
    template<size_t kFilterNum, size_t kFilterSize>
    class MatchCalculator final : public CorrectionCalculator<kFilterNum, kFilterSize> {
    public:
        static constexpr size_t kStartDecayIdx = 16, kEndDecayIdx = 128;

        MatchCalculator() : CorrectionCalculator<kFilterNum, kFilterSize>() {
            decays_.resize(kEndDecayIdx);
            const auto k1 = std::log(static_cast<float>(kStartDecayIdx));
            const auto k2 = std::log(static_cast<float>(kEndDecayIdx));
            const auto k3 = 1.f / (k2 - k1);
            for (size_t i = 0; i < kStartDecayIdx; ++i) {
                decays_[i] = 0.f;
            }
            for (size_t i = kStartDecayIdx; i < kEndDecayIdx; ++i) {
                decays_[i] = std::clamp((std::log(static_cast<float>(i)) - k1) * k3, 0.f, 1.f);
            }
        }

    private:
        std::vector<float> decays_{};

        void prepareCorrection(const size_t) override {
        }

        void updateCorrection(const size_t idx) override {
            auto &proto_res{CorrectionCalculator<kFilterNum, kFilterSize>::proto_res_};
            auto &biquad_res{CorrectionCalculator<kFilterNum, kFilterSize>::biquad_res_};
            auto &correction{CorrectionCalculator<kFilterNum, kFilterSize>::corrections_[idx]};

            for (size_t w_idx = kStartDecayIdx; w_idx < kEndDecayIdx; ++w_idx) {
                auto proto = proto_res[w_idx];
                auto biquad = biquad_res[w_idx];
                if (std::abs(biquad) < CorrectionCalculator<kFilterNum, kFilterSize>::kMinMagnitude) {
                    // ill condition, keep correction the same
                } else {
                    // normal condition
                    auto z = proto / biquad;
                    const auto k = decays_[w_idx];
                    correction[w_idx] *= std::polar(std::fma(k, std::abs(z) - 1.0f, 1.0f), k * std::arg(z));
                }
            }
            for (size_t w_idx = kEndDecayIdx; w_idx < correction.size(); ++w_idx) {
                auto proto = proto_res[w_idx];
                auto biquad = biquad_res[w_idx];
                if (std::abs(biquad) < CorrectionCalculator<kFilterNum, kFilterSize>::kMinMagnitude) {
                    // ill condition, keep correction the same
                } else {
                    // normal condition
                    correction[w_idx] *= (proto / biquad);
                }
            }
            // if a single correction is larger than 40 dB, scale back to 40 dB
            for (size_t w_idx = kStartDecayIdx; w_idx < correction.size(); ++w_idx) {
                if (std::max(std::abs(correction[w_idx].real()), std::abs(correction[w_idx].imag())) > 100.f) {
                    correction[w_idx] *= 100.f / std::abs(correction[w_idx]);
                }
            }
        }
    };
}
