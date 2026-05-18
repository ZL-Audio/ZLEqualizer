// Copyright (C) 2026 - zsliu98
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

namespace zldsp::filter {
    template <size_t kFilterNum, size_t kFilterSize>
    class CorrectionCalculator {
    public:
        CorrectionCalculator() = default;

        virtual ~CorrectionCalculator() = default;

        void prepare(const size_t num_bin) {
            w_prototype_.resize(num_bin);
            zldsp::filter::IdealBase<float>::calculateWs(w_prototype_);

            w_biquad_real_.resize(num_bin);
            w_biquad_imag_.resize(num_bin);
            zldsp::filter::TDFBase<float>::calculateWs(w_biquad_real_, w_biquad_imag_);

            proto_real_.resize(num_bin);
            proto_imag_.resize(num_bin);
            biquad_real_.resize(num_bin);
            biquad_imag_.resize(num_bin);

            for (auto& correction : corrections_real_) {
                correction.resize(num_bin);
                correction[0] = 1.f;
            }
            for (auto& correction : corrections_imag_) {
                correction.resize(num_bin);
            }
        }

        void update(std::array<TDF<float, kFilterSize>, kFilterNum>& tdfs,
                    std::array<Ideal<float, kFilterSize>, kFilterNum>& ideals,
                    const std::span<size_t> indices,
                    std::array<bool, kFilterNum>& update_flags) {
            // update filter corrections
            for (const size_t& i : indices) {
                if (!update_flags[i]) { continue; }
                std::ranges::fill(corrections_real_[i], 1.f);
                std::ranges::fill(corrections_imag_[i], 0.f);
                auto& ideal{ideals[i]};
                auto& tdf{tdfs[i]};
                const auto filter_num = ideal.getFilterNum();
                for (size_t idx = 0; idx < filter_num; ++idx) {
                    // update proto response
                    const auto proto_coeff = ideal.getCoeff()[idx];
                    IdealBase<float>::updateResponse(proto_coeff, w_prototype_,
                                                     proto_real_, proto_imag_);
                    // update biquad response
                    const auto biquad_coeff = tdf.getCoeff()[idx];
                    TDFBase<float>::updateResponse(biquad_coeff, w_biquad_real_, w_biquad_imag_,
                                                   biquad_real_, biquad_imag_);
                    // update correction
                    updateCorrection(i);
                }
            }
        }

        std::array<vector::aligned_vector<float>, kFilterNum>& getCorrectionsReal() {
            return corrections_real_;
        }

        std::array<vector::aligned_vector<float>, kFilterNum>& getCorrectionsImag() {
            return corrections_imag_;
        }

    protected:
        static constexpr float kMinMagnitude = 1e-8f, kMaxMagnitude = 1e8f;
        static constexpr float kMinMagSqr = 1e-16f, kMaxMagSqr = 1e16f;
        vector::aligned_vector<float> w_prototype_;
        vector::aligned_vector<float> w_biquad_real_, w_biquad_imag_;

        vector::aligned_vector<float> proto_real_, proto_imag_;
        vector::aligned_vector<float> biquad_real_, biquad_imag_;

        std::array<vector::aligned_vector<float>, kFilterNum> corrections_real_{};
        std::array<vector::aligned_vector<float>, kFilterNum> corrections_imag_{};

        virtual void prepareCorrection(size_t num_bin) = 0;

        virtual void updateCorrection(size_t idx) = 0;
    };
}
