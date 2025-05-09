// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "../../container/array.hpp"
#include "fir_base.hpp"

namespace zldsp::filter {
    /**
     * an FIR which corrects the magnitude responses of IIR filters to prototype filters
     * and minimizes phase responses at high-end
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterNum the number of filters
     * @tparam FilterSize the size of each filter
     */
    template<typename FloatType, size_t FilterNum, size_t FilterSize>
    class MixedCorrection final : public FIRBase<FloatType, 11>  {
    public:
        static constexpr size_t kStartMixIdx = 4, kEndMixIdx = 512;
        static constexpr double kDecayMultiplier = 0.98;

        MixedCorrection(std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iir,
                        std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
                        zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &indices,
                        std::array<bool, FilterNum> &mask,
                        std::vector<std::complex<FloatType> > &w1,
                        std::vector<std::complex<FloatType> > &w2)

            : iir_fs_(iir), ideal_fs_(ideal),
              filter_indices_(indices), bypass_mask_(mask),
              wis1_(w1), wis2_(w2) {
        }

        void setToUpdate() { to_update_.store(true); }

        size_t getCorrectionSize() const { return corrections_.size(); }

    private:
        std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iir_fs_;
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal_fs_;
        zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &filter_indices_;
        std::array<bool, FilterNum> &bypass_mask_;
        std::atomic<bool> to_update_{true};

        std::vector<std::complex<FloatType> > iir_total_response_, ideal_total_response_;
        // mixed corrections
        std::vector<std::complex<float> > corrections_{};
        std::vector<std::complex<FloatType> > &wis1_, &wis2_;
        std::vector<FloatType> correction_mix_{};

        void setOrder(const size_t channel_num, const size_t order) override {
            FIRBase<FloatType, 11>::setFFTOrder(channel_num, order);

            corrections_.resize(FIRBase<FloatType, 11>::num_bins_);
            correction_mix_.resize(FIRBase<FloatType, 11>::num_bins_);

            FIRBase<FloatType, 11>::reset();

            double mix = kDecayMultiplier;
            for (size_t i = 0; i < kStartMixIdx; ++i) {
                correction_mix_[i] = FloatType(1);
            }
            for (size_t i = kStartMixIdx; i < kEndMixIdx; ++i) {
                correction_mix_[i] = static_cast<FloatType>(mix);
                mix *= kDecayMultiplier;
            }
            for (size_t i = kEndMixIdx; i < correction_mix_.size(); ++i) {
                correction_mix_[i] = FloatType(0);
            }
        }

        void processSpectrum() override {
            update();
            auto *cdata = reinterpret_cast<std::complex<float> *>(FIRBase<FloatType, 11>::fft_data_.data());
            zldsp::vector::multiply(cdata + kStartMixIdx, corrections_.data() + kStartMixIdx, corrections_.size() - kStartMixIdx);
        }

        void update() {
            // check whether a filter has been updated
            bool need_to_update{false};
            for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                const auto i = filter_indices_[idx];
                if (!bypass_mask_[i]) {
                    need_to_update = need_to_update || ideal_fs_[i].updateMixPhaseResponse(
                                       wis1_, kStartMixIdx, kEndMixIdx, correction_mix_);
                    need_to_update = need_to_update || iir_fs_[i].updateResponse(wis2_);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (need_to_update || to_update_.exchange(false)) {
                bool has_been_updated = false;
                for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                    const auto i = filter_indices_[idx];
                    if (!bypass_mask_[i]) {
                        const auto &ideal_response = ideal_fs_[i].getResponse();
                        const auto &iir_response = iir_fs_[i].getResponse();
                        if (!has_been_updated) {
                            for (size_t j = kStartMixIdx; j < corrections_.size() - 1; ++j) {
                                corrections_[j] = static_cast<std::complex<float>>(ideal_response[j] / iir_response[j]);
                            }
                            has_been_updated = true;
                        } else {
                            for (size_t j = kStartMixIdx; j < corrections_.size() - 1; ++j) {
                                corrections_[j] *= static_cast<std::complex<float>>(ideal_response[j] / iir_response[j]);
                            }
                        }
                    }
                }
                if (!has_been_updated) {
                    std::fill(corrections_.begin(), corrections_.end(), std::complex(1.f, 0.f));
                } else {
                    // remove all infinity & NaN
                    for (size_t j = kStartMixIdx; j < corrections_.size() - 1; ++j) {
                        if (!std::isfinite(corrections_[j].real()) || !std::isfinite(corrections_[j].imag())
                            || std::abs(corrections_[j].real()) > 10000.f || std::abs(corrections_[j].imag()) > 10000.f) {
                            corrections_[j] = std::complex(1.f, 0.f);
                        }
                    }
                    corrections_.end()[-1] = std::abs(corrections_.end()[-2]);
                }
            }
        }
    };
}
