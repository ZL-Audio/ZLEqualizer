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
    class ZeroCalculator final : public CorrectionCalculator<kFilterNum, kFilterSize> {
    public:
        ZeroCalculator() : CorrectionCalculator<kFilterNum, kFilterSize>() {
        }

    private:
        void prepareCorrection(const size_t) override {
        }

        void updateCorrection(const size_t idx) override {
            auto &proto_res{CorrectionCalculator<kFilterNum, kFilterSize>::proto_res_};
            auto &biquad_res{CorrectionCalculator<kFilterNum, kFilterSize>::biquad_res_};
            auto &correction{CorrectionCalculator<kFilterNum, kFilterSize>::corrections_[idx]};

            for (size_t w_idx = 1; w_idx < correction.size(); ++w_idx) {
                auto proto = proto_res[w_idx];
                auto biquad = biquad_res[w_idx];
                if (std::abs(biquad) < CorrectionCalculator<kFilterNum, kFilterSize>::kMinMagnitude) {
                    // ill condition, keep correction the same
                } else {
                    // normal condition
                    correction[w_idx] *= std::complex(std::abs(proto), 0.f) / biquad;
                }
            }
            // if a single correction is larger than 40 dB, scale back to 40 dB
            for (size_t w_idx = 1; w_idx < correction.size(); ++w_idx) {
                if (std::max(std::abs(correction[w_idx].real()), std::abs(correction[w_idx].imag())) > 100.f) {
                    correction[w_idx] *= 100.f / std::abs(correction[w_idx]);
                }
            }
        }
    };
}
