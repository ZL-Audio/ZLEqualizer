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

namespace zlFilter {
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
        static constexpr size_t startMixIdx = 4, endMixIdx = 512;
        static constexpr double decayMultiplier = 0.98;

        MixedCorrection(std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iir,
                        std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
                        zlContainer::FixedMaxSizeArray<size_t, FilterNum> &indices,
                        std::array<bool, FilterNum> &mask,
                        std::vector<std::complex<FloatType> > &w1,
                        std::vector<std::complex<FloatType> > &w2)

            : iirFs(iir), idealFs(ideal),
              filterIndices(indices), bypassMask(mask),
              wis1(w1), wis2(w2) {
        }

        void setToUpdate() { toUpdate.store(true); }

        size_t getCorrectionSize() const { return corrections.size(); }

    private:
        std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iirFs;
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &idealFs;
        zlContainer::FixedMaxSizeArray<size_t, FilterNum> &filterIndices;
        std::array<bool, FilterNum> &bypassMask;
        std::atomic<bool> toUpdate{true};

        std::vector<std::complex<FloatType> > iirTotalResponse, idealTotalResponse;
        // mixed corrections
        std::vector<std::complex<float> > corrections{};
        std::vector<std::complex<FloatType> > &wis1, &wis2;
        std::vector<FloatType> correctionMix{};

        void setOrder(const size_t channelNum, const size_t order) override {
            FIRBase<FloatType, 11>::setFFTOrder(channelNum, order);

            corrections.resize(FIRBase<FloatType, 11>::numBins);
            correctionMix.resize(FIRBase<FloatType, 11>::numBins);

            FIRBase<FloatType, 11>::reset();

            double mix = decayMultiplier;
            for (size_t i = 0; i < startMixIdx; ++i) {
                correctionMix[i] = FloatType(1);
            }
            for (size_t i = startMixIdx; i < endMixIdx; ++i) {
                correctionMix[i] = static_cast<FloatType>(mix);
                mix *= decayMultiplier;
            }
            for (size_t i = endMixIdx; i < correctionMix.size(); ++i) {
                correctionMix[i] = FloatType(0);
            }
        }

        void processSpectrum() override {
            update();
            auto *cdata = reinterpret_cast<std::complex<float> *>(FIRBase<FloatType, 11>::fftData.data());
            zlVector::multiply(cdata + startMixIdx, corrections.data() + startMixIdx, corrections.size() - startMixIdx);
        }

        void update() {
            // check whether a filter has been updated
            bool needToUpdate{false};
            for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                const auto i = filterIndices[idx];
                if (!bypassMask[i]) {
                    needToUpdate = needToUpdate || idealFs[i].updateMixPhaseResponse(
                                       wis1, startMixIdx, endMixIdx, correctionMix);
                    needToUpdate = needToUpdate || iirFs[i].updateResponse(wis2);
                }
            }
            // if a filter has been updated or the correction has to be updated
            if (needToUpdate || toUpdate.exchange(false)) {
                bool hasBeenUpdated = false;
                for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                    const auto i = filterIndices[idx];
                    if (!bypassMask[i]) {
                        const auto &idealResponse = idealFs[i].getResponse();
                        const auto &iirResponse = iirFs[i].getResponse();
                        if (!hasBeenUpdated) {
                            for (size_t j = startMixIdx; j < corrections.size() - 1; ++j) {
                                corrections[j] = static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                            hasBeenUpdated = true;
                        } else {
                            for (size_t j = startMixIdx; j < corrections.size() - 1; ++j) {
                                corrections[j] *= static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                        }
                    }
                }
                if (!hasBeenUpdated) {
                    std::fill(corrections.begin(), corrections.end(), std::complex(1.f, 0.f));
                } else {
                    // remove all infinity & NaN
                    for (size_t j = startMixIdx; j < corrections.size() - 1; ++j) {
                        if (!std::isfinite(corrections[j].real()) || !std::isfinite(corrections[j].imag())
                            || std::abs(corrections[j].real()) > 10000.f || std::abs(corrections[j].imag()) > 10000.f) {
                            corrections[j] = std::complex(1.f, 0.f);
                        }
                    }
                    corrections.end()[-1] = std::abs(corrections.end()[-2]);
                }
            }
        }
    };
}
