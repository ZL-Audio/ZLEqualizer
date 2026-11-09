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
            zldsp::filter::IdealBase<double>::calculateWs(w_prototype_);

            w_biquad_real_.resize(num_bin);
            w_biquad_imag_.resize(num_bin);
            zldsp::filter::TDFBase<double>::calculateWs(w_biquad_real_, w_biquad_imag_);

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
            for (auto& correction : output_real_) { correction.assign(num_bin, 1.f); }
            for (auto& correction : output_imag_) { correction.assign(num_bin, 0.f); }
        }

        /**
         * update the corrections for matching digital and prototype filters
         * call prepare before updating; each pair must have current coefficients for the same
         * parameters and sample rate, including the same scale and coverage for scaled passes.
         * @param tdfs the digital filters
         * @param ideals the matching prototype filters
         * @param indices filter indices in [0, kFilterNum)
         * @param update_flags whether each filter needs an update
         */
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
#if ZL_ENABLE_FILTER_SCALED_PASS
                if (isScaledPass(ideal.getFilterType())) {
                    const auto& proto = ideal.getScaledCoefficients();
                    const auto& digital = tdf.getScaledCoefficients();
                    for (size_t group = 0; group < proto.group_count; ++group) {
                        std::ranges::fill(proto_real_, 1.0);
                        std::ranges::fill(proto_imag_, 0.0);
                        std::ranges::fill(biquad_real_, 1.0);
                        std::ranges::fill(biquad_imag_, 0.0);
                        for (size_t idx = 0; idx < proto.first_count; ++idx) {
                            if (proto.first_groups[idx] != group) { continue; }
                            IdealBase<double>::template updateResponse<false>(proto.first[idx], w_prototype_,
                                                                              proto_real_, proto_imag_);
                            TDFBase<double>::template updateResponse<false>(digital.first[idx], w_biquad_real_,
                                                                            w_biquad_imag_, biquad_real_, biquad_imag_);
                        }
                        for (size_t idx = 0; idx < proto.second_count; ++idx) {
                            if (proto.second_groups[idx] != group) { continue; }
                            IdealBase<double>::template updateResponse<false>(proto.second[idx], w_prototype_,
                                                                              proto_real_, proto_imag_);
                            TDFBase<double>::template updateResponse<false>(digital.second[idx], w_biquad_real_,
                                                                            w_biquad_imag_, biquad_real_, biquad_imag_);
                        }
                        updateCorrection(i);
                    }
                } else
#endif
                {
                    const auto filter_num = ideal.getFilterNum();
                    for (size_t idx = 0; idx < filter_num; ++idx) {
                        if (ideal.usesFirstOrderSections()) {
                            const auto& c = ideal.getCoeff()[idx];
                            IdealBase<double>::updateResponse(std::array<double, 3>{c[0], c[2], c[3]},
                                                              w_prototype_, proto_real_, proto_imag_);
                        } else {
                            IdealBase<double>::updateResponse(ideal.getCoeff()[idx], w_prototype_,
                                                              proto_real_, proto_imag_);
                        }
                        TDFBase<double>::updateResponse(tdf.getCoeff()[idx], w_biquad_real_, w_biquad_imag_,
                                                        biquad_real_, biquad_imag_);
                        updateCorrection(i);
                    }
                }
                for (size_t bin = 0; bin < w_prototype_.size(); ++bin) {
                    output_real_[i][bin] = static_cast<float>(corrections_real_[i][bin]);
                    output_imag_[i][bin] = static_cast<float>(corrections_imag_[i][bin]);
                }
            }
        }

        std::array<vector::aligned_vector<float>, kFilterNum>& getCorrectionsReal() {
            return output_real_;
        }

        std::array<vector::aligned_vector<float>, kFilterNum>& getCorrectionsImag() {
            return output_imag_;
        }

    protected:
        static constexpr double kMinMagnitude = 1e-8, kMaxMagnitude = 1e8;
        static constexpr double kMinMagSqr = 1e-16, kMaxMagSqr = 1e16;
        vector::aligned_vector<double> w_prototype_;
        vector::aligned_vector<double> w_biquad_real_, w_biquad_imag_;

        vector::aligned_vector<double> proto_real_, proto_imag_;
        vector::aligned_vector<double> biquad_real_, biquad_imag_;

        std::array<vector::aligned_vector<double>, kFilterNum> corrections_real_{};
        std::array<vector::aligned_vector<double>, kFilterNum> corrections_imag_{};

        std::array<vector::aligned_vector<float>, kFilterNum> output_real_{}, output_imag_{};

        virtual void updateCorrection(size_t idx) = 0;
    };
}
