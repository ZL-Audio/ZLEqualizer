// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "fir_base.hpp"
#include "fir_helper.hpp"
#include "../iir_filter/tdf/tdf.hpp"
#include "../ideal_filter/ideal.hpp"

namespace zldsp::filter {
    static constexpr size_t kMatchDefaultOrder = 9;

    template<typename FloatType, size_t kFilterNum, size_t kFilterSize>
    class MatchCorrection final : FIRBase<FloatType, kMatchDefaultOrder> {
    public:
        MatchCorrection() : FIRBase<FloatType, kMatchDefaultOrder>() {
        }

        void prepareBuffer(std::span<TDF<float, kFilterSize> > tdfs,
                           std::span<Ideal<float, kFilterSize> > ideals,
                           std::span<bool> to_update,
                           std::span<size_t> on_indices) {
            if (on_indices.empty()) {
                std::fill(total_correction_.begin(), total_correction_.end(), std::complex(1.f, 0.f));
            }
            // update filter corrections
            for (size_t &i: on_indices) {
                if (!to_update[i]) {
                    continue;
                }
                to_update[i] = false;
                // update ideal filter response
                {
                    auto &ideal{ideals[i]};
                    const auto filter_num = ideal.getFilterNum();
                    for (size_t idx = 0; idx < filter_num; ++idx) {
                        const auto coeff = ideal.getCoeff()[idx];
                        if (idx == 0) {
                            for (size_t w_idx = 1; w_idx < w_prototype_.size(); ++w_idx) {
                                proto_res_[w_idx] = zldsp::filter::IdealBase<float>::getResponse(
                                    coeff, w_prototype_[w_idx]);
                            }
                        } else {
                            for (size_t w_idx = 1; w_idx < w_prototype_.size(); ++w_idx) {
                                proto_res_[w_idx] *= zldsp::filter::IdealBase<float>::getResponse(
                                    coeff, w_prototype_[w_idx]);
                            }
                        }
                    }
                }
                // update tdf filter response
                {
                    auto &tdf{tdfs[i]};
                    const auto filter_num = tdf.getFilterNum();
                    for (size_t idx = 0; idx < filter_num; ++idx) {
                        const auto coeff = tdf.getCoeff()[idx];
                        if (idx == 0) {
                            for (size_t w_idx = 1; w_idx < w_prototype_.size(); ++w_idx) {
                                biquad_res_[w_idx] = zldsp::filter::TDFBase<float>::getResponse(
                                    coeff, w_prototype_[w_idx]);
                            }
                        } else {
                            for (size_t w_idx = 1; w_idx < w_prototype_.size(); ++w_idx) {
                                biquad_res_[w_idx] *= zldsp::filter::TDFBase<float>::getResponse(
                                    coeff, w_prototype_[w_idx]);
                            }
                        }
                    }
                }
                // update match correction
                {
                    auto &correction{corrections_[i]};
                    for (size_t w_idx = 1; w_idx < correction.size(); ++w_idx) {
                        auto proto_res = proto_res_[w_idx];
                        auto biquad_res = biquad_res_[w_idx];
                        const auto biquad_mag = std::abs(biquad_res);
                        if (biquad_mag < kMinMagnitude) {
                            if (std::abs(proto_res) < kMinMagnitude) { // 0 / 0 condition
                                correction[w_idx] = std::complex(1.f, 0.f);
                            } else { // x / 0 condition, use the previous correction
                                correction[w_idx] = correction[w_idx - 1];
                            }
                        } else { // normal condition
                            correction[w_idx] = proto_res / biquad_res;
                        }
                    }
                }
            }
            // multiply corrections
            bool is_first = true;
            for (size_t &i: on_indices) {
                if (is_first) {
                    total_correction_ = corrections_[i];
                    is_first = false;
                } else {
                    total_correction_ *= corrections_[i];
                }
                for (size_t w_idx = 1; w_idx < total_correction_.size(); ++w_idx) {
                    if (std::abs(total_correction_[w_idx]) > kMaxMagnitude) {
                        total_correction_[w_idx] = total_correction_[w_idx - 1];
                    }
                }
            }
        }

    private:
        static constexpr float kMinMagnitude = 1e-8f, kMaxMagnitude = 1e8f;
        kfr::univector<std::complex<float> > w_prototype_{}, w_biquad_{};
        kfr::univector<std::complex<float> > proto_res_{}, biquad_res_{};
        std::array<kfr::univector<std::complex<float> >, kFilterNum> corrections_{};
        kfr::univector<std::complex<float> > total_correction_{};

        void setOrder(size_t num_channels, size_t order) override {
            FIRBase<FloatType, kMatchDefaultOrder>::setFFTOrder(num_channels, order);

            const auto num_bin = FIRBase<FloatType, kMatchDefaultOrder>::num_bins_;

            w_prototype_.resize(num_bin);
            calculateWsForPrototype<float>(w_prototype_);

            w_biquad_.resize(num_bin);
            calculateWsForBiquad<float>(w_biquad_);

            proto_res_.resize(num_bin);
            biquad_res_.resize(num_bin);

            for (auto &correction: corrections_) {
                correction.resize(num_bin);
                correction[0] = std::complex(1.f, 0.f);
            }
            total_correction_.resize(num_bin);
            total_correction_[0] = std::complex(1.f, 0.f);
        }

        void processSpectrum() override {
            auto *cdata = reinterpret_cast<std::complex<float> *>(FIRBase<FloatType, kMatchDefaultOrder>::fft_data_.data());
            auto c_vector = kfr::make_univector(cdata, total_correction_.size());
            c_vector = c_vector * total_correction_;
        }
    };
}
