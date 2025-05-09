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
     * @tparam defaultFFTOrder the default FFT order for 44100/48000 Hz input
     */
    template<typename FloatType, size_t FilterNum, size_t FilterSize, size_t defaultFFTOrder = 13>
    class FIR final : public FIRBase<FloatType, defaultFFTOrder> {
    public:
        FIR(std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
            zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &indices,
            std::array<bool, FilterNum> &mask,
            std::vector<std::complex<FloatType> > &w1)

            : idealFs(ideal),
              filterIndices(indices), bypassMask(mask),
              wis1(w1) {
        }

        void setToUpdate() { toUpdate.store(true); }

        size_t getCorrectionSize() const { return corrections.size(); }

    private:
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &idealFs;
        zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &filterIndices;
        std::array<bool, FilterNum> &bypassMask;
        std::atomic<bool> toUpdate{true};

        std::vector<std::complex<FloatType> > idealTotalResponse;

        kfr::univector<float> corrections{}, dummyCorrections{};
        std::vector<std::complex<FloatType> > &wis1;

        void setOrder(const size_t channelNum, const size_t order) override {
            FIRBase<FloatType, defaultFFTOrder>::setFFTOrder(channelNum, order);

            corrections.resize(FIRBase<FloatType, defaultFFTOrder>::numBins);
            dummyCorrections.resize(FIRBase<FloatType, defaultFFTOrder>::numBins << 1);
            FIRBase<FloatType, defaultFFTOrder>::reset();
        }

        void processSpectrum() override {
            update();
            zldsp::vector::multiply(FIRBase<FloatType, defaultFFTOrder>::fftData.data(), dummyCorrections.data(), dummyCorrections.size());
        }

        void update() {
            // check whether a filter has been updated
            bool needToUpdate{false};
            for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                const auto i = filterIndices[idx];
                if (!bypassMask[i]) {
                    needToUpdate = needToUpdate || idealFs[i].updateZeroPhaseResponse(wis1);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (needToUpdate || toUpdate.exchange(false)) {
                bool hasBeenUpdated = false;
                for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                    const auto i = filterIndices[idx];
                    if (!bypassMask[i]) {
                        const auto &idealResponse = idealFs[i].getResponse();
                        if (!hasBeenUpdated) {
                            for (size_t j = 1; j < corrections.size(); ++j) {
                                corrections[j] = static_cast<float>(idealResponse[j].real());
                            }
                            hasBeenUpdated = true;
                        } else {
                            for (size_t j = 1; j < corrections.size(); ++j) {
                                corrections[j] *= static_cast<float>(idealResponse[j].real());
                            }
                        }
                    }
                }
                if (!hasBeenUpdated) {
                    std::fill(dummyCorrections.begin(), dummyCorrections.end(), 1.f);
                } else {
                    corrections[0] = corrections[1];
                    for (size_t j = 0; j < corrections.size(); ++j) {
                        dummyCorrections[j << 1] = corrections[j];
                        dummyCorrections[(j << 1) + 1] = corrections[j];
                    }
                }
            }
        }
    };
}
