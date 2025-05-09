// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../ideal_filter/ideal_filter.hpp"
#include "../../container/array.hpp"
#include "fir_base.hpp"

namespace zldsp::filter {
    /**
     * an FIR which has the magnitude responses of prototype filters and zero phase responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterNum the number of filters
     * @tparam FilterSize the size of each filter
     * @tparam DefaultFFTOrder the default FFT order for 44100/48000 Hz input
     */
    template<typename FloatType, size_t FilterNum, size_t FilterSize, size_t DefaultFFTOrder = 13>
    class FIR final : public FIRBase<FloatType, DefaultFFTOrder> {
    public:
        FIR(std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
            zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &indices,
            std::array<bool, FilterNum> &mask,
            std::vector<std::complex<FloatType> > &w1)

            : ideal_fs_(ideal),
              filter_indices_(indices), bypass_mask_(mask),
              wis1_(w1) {
        }

        void setToUpdate() { to_update_.store(true); }

        size_t getCorrectionSize() const { return corrections_.size(); }

    private:
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal_fs_;
        zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &filter_indices_;
        std::array<bool, FilterNum> &bypass_mask_;
        std::atomic<bool> to_update_{true};

        std::vector<std::complex<FloatType> > ideal_total_response_;

        kfr::univector<float> corrections_{}, dummy_corrections_{};
        std::vector<std::complex<FloatType> > &wis1_;

        void setOrder(const size_t channel_num, const size_t order) override {
            FIRBase<FloatType, DefaultFFTOrder>::setFFTOrder(channel_num, order);

            corrections_.resize(FIRBase<FloatType, DefaultFFTOrder>::num_bins_);
            dummy_corrections_.resize(FIRBase<FloatType, DefaultFFTOrder>::num_bins_ << 1);
            FIRBase<FloatType, DefaultFFTOrder>::reset();
        }

        void processSpectrum() override {
            update();
            zldsp::vector::multiply(FIRBase<FloatType, DefaultFFTOrder>::fft_data_.data(), dummy_corrections_.data(), dummy_corrections_.size());
        }

        void update() {
            // check whether a filter has been updated
            bool need_to_update{false};
            for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                const auto i = filter_indices_[idx];
                if (!bypass_mask_[i]) {
                    need_to_update = need_to_update || ideal_fs_[i].updateZeroPhaseResponse(wis1_);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (need_to_update || to_update_.exchange(false)) {
                bool has_been_updated = false;
                for (size_t idx = 0; idx < filter_indices_.size(); ++idx) {
                    const auto i = filter_indices_[idx];
                    if (!bypass_mask_[i]) {
                        const auto &idealResponse = ideal_fs_[i].getResponse();
                        if (!has_been_updated) {
                            for (size_t j = 1; j < corrections_.size(); ++j) {
                                corrections_[j] = static_cast<float>(idealResponse[j].real());
                            }
                            has_been_updated = true;
                        } else {
                            for (size_t j = 1; j < corrections_.size(); ++j) {
                                corrections_[j] *= static_cast<float>(idealResponse[j].real());
                            }
                        }
                    }
                }
                if (!has_been_updated) {
                    std::fill(dummy_corrections_.begin(), dummy_corrections_.end(), 1.f);
                } else {
                    corrections_[0] = corrections_[1];
                    for (size_t j = 0; j < corrections_.size(); ++j) {
                        dummy_corrections_[j << 1] = corrections_[j];
                        dummy_corrections_[(j << 1) + 1] = corrections_[j];
                    }
                }
            }
        }
    };
}
