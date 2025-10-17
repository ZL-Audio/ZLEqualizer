// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../vector/vector.hpp"
#include "../iir_filter/tdf/tdf.hpp"
#include "../ideal_filter/ideal.hpp"
#include "fir_helper.hpp"

namespace zldsp::filter {
    template <size_t kFilterNum, size_t kFilterSize>
    class CorrectionCalculator {
    public:
        CorrectionCalculator() = default;

        virtual ~CorrectionCalculator() = default;

        void prepare(const size_t num_bin) {
            w_prototype_.resize(num_bin);
            calculateWsForPrototype<float>(w_prototype_);

            w_biquad_.resize(num_bin);
            calculateWsForBiquad<float>(w_biquad_);

            proto_res_.resize(num_bin);
            biquad_res_.resize(num_bin);

            for (auto& correction : corrections_) {
                correction.resize(num_bin);
                correction[0] = std::complex(1.f, 0.f);
            }
        }

        void update(std::array<TDF < float, kFilterSize>, kFilterNum> &tdfs,
                    std::array<Ideal < float, kFilterSize>, kFilterNum> &ideals,
                    const std::span<size_t> indices,
                    std::array<bool, kFilterNum>& update_flags) {
            // update filter corrections
            for (const size_t& i : indices) {
                if (!update_flags[i]) { continue; }
                std::fill(corrections_[i].begin(), corrections_[i].end(), std::complex(1.f, 0.f));
                auto& ideal{ideals[i]};
                auto& tdf{tdfs[i]};
                const auto filter_num = ideal.getFilterNum();
                for (size_t idx = 0; idx < filter_num; ++idx) {
                    // update proto response
                    const auto proto_coeff = ideal.getCoeff()[idx];
                    for (size_t w_idx = 1; w_idx < w_prototype_.size(); ++w_idx) {
                        proto_res_[w_idx] = IdealBase::getResponse(proto_coeff, w_prototype_[w_idx]);
                    }
                    // update biquad response
                    const auto biquad_coeff = tdf.getCoeff()[idx];
                    for (size_t w_idx = 1; w_idx < w_biquad_.size(); ++w_idx) {
                        biquad_res_[w_idx] = TDFBase<float>::getResponse(biquad_coeff, w_biquad_[w_idx]);
                    }
                    // update correction
                    updateCorrection(i);
                }
            }
        }

        std::array<kfr::univector<std::complex<float>>, kFilterNum>& getCorrections() {
            return corrections_;
        }

    protected:
        static constexpr float kMinMagnitude = 1e-8f, kMaxMagnitude = 1e8f;
        kfr::univector<std::complex<float>> w_prototype_{}, w_biquad_{};
        kfr::univector<std::complex<float>> proto_res_{}, biquad_res_{};
        std::array<kfr::univector<std::complex<float>>, kFilterNum> corrections_{};

        virtual void prepareCorrection(size_t num_bin) = 0;

        virtual void updateCorrection(size_t idx) = 0;
    };
}
