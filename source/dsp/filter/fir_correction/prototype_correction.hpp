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
        static constexpr size_t defaultFFTOrder = 10;
        static constexpr size_t startDecayIdx = 2, endDecayIdx = 8;

        PrototypeCorrection(std::array<IIRIdle<FloatType, FilterSize>, FilterNum> &iir,
                            std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal,
                            zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &indices,
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
        zldsp::container::FixedMaxSizeArray<size_t, FilterNum> &filterIndices;
        std::array<bool, FilterNum> &bypassMask;
        std::atomic<bool> toUpdate{true};

        std::vector<std::complex<FloatType> > iirTotalResponse, idealTotalResponse;
        // prototype corrections
        std::vector<std::complex<float> > corrections{};
        std::vector<std::complex<FloatType> > &wis1, &wis2;
        float deltaDecay{0.f};

        void setOrder(const size_t channelNum, const size_t order) override {
            FIRBase<FloatType, 10>::setFFTOrder(channelNum, order);

            corrections.resize(FIRBase<FloatType, 10>::numBins);

            deltaDecay = 1.f / static_cast<float>(endDecayIdx - startDecayIdx);

            FIRBase<FloatType, 10>::reset();
        }

        void processSpectrum() override {
            update();
            auto *cdata = reinterpret_cast<std::complex<float> *>(FIRBase<FloatType, 10>::fftData.data());
            zldsp::vector::multiply(cdata + startDecayIdx, corrections.data() + startDecayIdx, corrections.size() - startDecayIdx);
        }

        void update() {
            // check whether a filter has been updated
            bool needToUpdate{false};
            for (size_t idx = 0; idx < filterIndices.size(); ++idx) {
                const auto i = filterIndices[idx];
                if (!bypassMask[i]) {
                    needToUpdate = needToUpdate || idealFs[i].updateResponse(wis1);
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
                            for (size_t j = startDecayIdx; j < corrections.size() - 1; ++j) {
                                corrections[j] = static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                            hasBeenUpdated = true;
                        } else {
                            for (size_t j = startDecayIdx; j < corrections.size() - 1; ++j) {
                                corrections[j] *= static_cast<std::complex<float>>(idealResponse[j] / iirResponse[j]);
                            }
                        }
                    }
                }
                if (!hasBeenUpdated) {
                    std::fill(corrections.begin(), corrections.end(), std::complex(1.f, 0.f));
                } else {
                    // remove all infinity & NaN
                    for (size_t j = startDecayIdx; j < corrections.size() - 1; ++j) {
                        if (!std::isfinite(corrections[j].real()) || !std::isfinite(corrections[j].imag())
                            || std::abs(corrections[j].real()) > 10000.f || std::abs(corrections[j].imag()) > 10000.f) {
                            corrections[j] = std::complex(1.f, 0.f);
                        }
                    }
                    float decay = 0.f;
                    for (size_t j = startDecayIdx; j < endDecayIdx; ++j) {
                        corrections[j] = std::polar<float>(std::abs(corrections[j]) * decay + (1.f - decay),
                                                           std::arg(corrections[j]) * decay);
                        decay += deltaDecay;
                    }
                    corrections.end()[-1] = std::abs(corrections.end()[-2]);
                }
            }
        }
    };
}
