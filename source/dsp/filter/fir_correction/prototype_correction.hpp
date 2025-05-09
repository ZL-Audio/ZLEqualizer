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
     * an FIR which corrects the responses of IIR filters to prototype filters
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterNum the number of filters
     * @tparam FilterSize the size of each filter
     */
    template<typename FloatType, size_t FilterNum, size_t FilterSize>
    class PrototypeCorrection : public FIRBase<FloatType, 10> {
    public:
        static constexpr size_t kDefaultFFTOrder = 10;
        static constexpr size_t kStartDecayIdx = 2, kEndDecayIdx = 8;

        PrototypeCorrection(std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iir,
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
        // prototype corrections
        std::vector<std::complex<float> > corrections_{};
        std::vector<std::complex<FloatType> > &wis1_, &wis2_;
        float delta_decay_{0.f};

        void setOrder(const size_t channel_num, const size_t order) override {
            FIRBase<FloatType, 10>::setFFTOrder(channel_num, order);

            corrections_.resize(FIRBase<FloatType, 10>::num_bins_);

            delta_decay_ = 1.f / static_cast<float>(kEndDecayIdx - kStartDecayIdx);

            FIRBase<FloatType, 10>::reset();
        }

        void processSpectrum() override {
            update();
            auto *cdata = reinterpret_cast<std::complex<float> *>(FIRBase<FloatType, 10>::fft_data_.data());
            zldsp::vector::multiply(cdata + kStartDecayIdx, corrections_.data() + kStartDecayIdx, corrections_.size() - kStartDecayIdx);
        }

        void update() {
            // check whether a filter has been updated
            bool need_to_update{false};
            for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                const auto i = filter_indices_[idx];
                if (!bypass_mask_[i]) {
                    need_to_update = need_to_update || ideal_fs_[i].updateResponse(wis1_);
                    need_to_update = need_to_update || iir_fs_[i].updateResponse(wis2_);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (need_to_update || to_update_.exchange(false)) {
                bool has_been_updated = false;
                for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                    const auto i = filter_indices_[idx];
                    if (!bypass_mask_[i]) {
                        const auto &idealResponse = ideal_fs_[i].getResponse();
                        const auto &iirResponse = iir_fs_[i].getResponse();
                        if (!has_been_updated) {
                            for (size_t j = kStartDecayIdx; j < corrections_.size() - 1; ++j) {
                                corrections_[j] = static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                            has_been_updated = true;
                        } else {
                            for (size_t j = kStartDecayIdx; j < corrections_.size() - 1; ++j) {
                                corrections_[j] *= static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                        }
                    }
                }
                if (!has_been_updated) {
                    std::fill(corrections_.begin(), corrections_.end(), std::complex(1.f, 0.f));
                } else {
                    // remove all infinity & NaN
                    for (size_t j = kStartDecayIdx; j < corrections_.size() - 1; ++j) {
                        if (!std::isfinite(corrections_[j].real()) || !std::isfinite(corrections_[j].imag())
                            || std::abs(corrections_[j].real()) > 10000.f || std::abs(corrections_[j].imag()) > 10000.f) {
                            corrections_[j] = std::complex(1.f, 0.f);
                        }
                    }
                    float decay = 0.f;
                    for (size_t j = kStartDecayIdx; j < kEndDecayIdx; ++j) {
                        corrections_[j] = std::polar<float>(std::abs(corrections_[j]) * decay + (1.f - decay),
                                                           std::arg(corrections_[j]) * decay);
                        decay += delta_decay_;
                    }
                    corrections_.end()[-1] = std::abs(corrections_.end()[-2]);
                }
            }
        }
    };
}
