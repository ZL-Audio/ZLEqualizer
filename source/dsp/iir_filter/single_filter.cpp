// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_filter.hpp"

namespace zlIIR {
    template<typename FloatType>
    void Filter<FloatType>::reset() {
        if (toReset.exchange(false)) {
            for (size_t i = 0; i < filterNum.load(); ++i) {
                filters[i].reset();
            }
            for (size_t i = 0; i < filterNum.load(); ++i) {
                svfFilters[i].reset();
            }
            bypassNextBlock.store(true);
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        processSpec = spec;
        numChannels.store(spec.numChannels);
        sampleRate.store(static_cast<float>(spec.sampleRate));
        for (auto &f: filters) {
            f.prepare(spec);
        }
        for (auto &f: svfFilters) {
            f.prepare(spec);
        }
        setOrder(order.load());
    }

    template<typename FloatType>
    void Filter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer, bool isBypassed) {
        const auto nextUseSVF = useSVF.load();
        if (currentUseSVF != nextUseSVF) {
            currentUseSVF = nextUseSVF;
            toReset.store(true);
            toUpdatePara.store(true);
        }
        reset();
        updateParas();
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        context.isBypassed = isBypassed || bypassNextBlock.exchange(false);
        if (!currentUseSVF) {
            for (size_t i = 0; i < filterNum.load(); ++i) {
                filters[i].process(context);
            }
        } else {
            for (size_t i = 0; i < filterNum.load(); ++i) {
                svfFilters[i].process(context);
            }
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::setFreq(const FloatType x, const bool update) {
        const auto diff = std::max(static_cast<double>(x), freq.load()) /
                          std::min(static_cast<double>(x), freq.load());
        if (std::log10(diff) >= 2 && update) {
            toReset.store(true);
        }
        freq.store(static_cast<double>(x));
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void Filter<FloatType>::setGain(const FloatType x, const bool update) {
        gain.store(static_cast<double>(x));
        switch (filterType.load()) {
            case peak:
            case bandShelf:
            case lowShelf:
            case highShelf:
            case tiltShelf: {
                if (update) { toUpdatePara.store(true); }
                break;
            }
            case lowPass:
            case highPass:
            case notch:
            case bandPass: {
                break;
            }
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::setQ(const FloatType x, const bool update) {
        q.store(static_cast<double>(x));
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void Filter<FloatType>::setFilterType(const FilterType x, const bool update) {
        toReset.store(true);
        filterType.store(x);
        if (update) { toUpdatePara.store(true); }
    }

    template<typename FloatType>
    void Filter<FloatType>::setOrder(const size_t x, const bool update) {
        order.store(x);
        if (update) {
            toReset.store(true);
            toUpdatePara.store(true);
        }
    }

    template<typename FloatType>
    bool Filter<FloatType>::updateParas() {
        if (toUpdatePara.exchange(false)) {
            filterNum.store(DesignFilter::updateCoeff(filterType.load(),
                                                      freq.load(), processSpec.sampleRate,
                                                      gain.load(), q.load(), order.load(), coeffs)); {
                farbot::RealtimeObject<
                    std::array<coeff33, 16>,
                    farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                    farbot::ThreadType::realtime> rrcentCoeffs(recentCoeffs);
                *rrcentCoeffs = coeffs;
            }
            magOutdated.store(true);
            if (!currentUseSVF) {
                for (size_t i = 0; i < filterNum.load(); i++) {
                    *filters[i].state = {
                        static_cast<FloatType>(std::get<1>(coeffs[i])[0]),
                        static_cast<FloatType>(std::get<1>(coeffs[i])[1]),
                        static_cast<FloatType>(std::get<1>(coeffs[i])[2]),
                        static_cast<FloatType>(std::get<0>(coeffs[i])[0]),
                        static_cast<FloatType>(std::get<0>(coeffs[i])[1]),
                        static_cast<FloatType>(std::get<0>(coeffs[i])[2])
                    };
                }
            } else {
                for (size_t i = 0; i < filterNum.load(); i++) {
                    svfFilters[i].updateFromBiquad(coeffs[i]);
                }
            }
            return true;
        }
        return false;
    }

    template<typename FloatType>
    bool Filter<FloatType>::updateParasForDBOnly() {
        if (toUpdatePara.exchange(false)) {
            filterNum.store(DesignFilter::updateCoeff(filterType.load(),
                                                      freq.load(), processSpec.sampleRate,
                                                      gain.load(), q.load(), order.load(), coeffs)); {
                farbot::RealtimeObject<
                    std::array<coeff33, 16>,
                    farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                    farbot::ThreadType::realtime> rrcentCoeffs(recentCoeffs);
                *rrcentCoeffs = coeffs;
            }
            magOutdated.store(true);
            return true;
        }
        return false;
    }

    template<typename FloatType>
    void Filter<FloatType>::addDBs(std::array<FloatType, frequencies.size()> &x, FloatType scale) {
        std::transform(x.begin(), x.end(), dBs.begin(), x.begin(),
                       [&scale](auto &c1, auto &c2) { return c1 + c2 * scale; });
    }

    template<typename FloatType>
    void Filter<FloatType>::addGains(std::array<FloatType, frequencies.size()> &x, FloatType scale) {
        std::transform(x.begin(), x.end(), gains.begin(), x.begin(),
                       [&scale](auto &c1, auto &c2) { return c1 + c2 * scale; });
    }

    template<typename FloatType>
    void Filter<FloatType>::updateDBs() {
        if (!magOutdated.exchange(false)) {
            return;
        }
        gains.fill(FloatType(1));
        std::array<double, frequencies.size()> singleMagnitudes{};
        juce::dsp::IIR::Coefficients<FloatType> dummyCoeff;
        farbot::RealtimeObject<
            std::array<coeff33, 16>,
            farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
            farbot::ThreadType::nonRealtime> rrcentCoeffs(recentCoeffs);
        for (size_t i = 0; i < filterNum.load(); i++) {
            dummyCoeff = {
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[0]),
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[1]),
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[2]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[0]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[1]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[2])
            };
            dummyCoeff.getMagnitudeForFrequencyArray(&frequencies[0], &singleMagnitudes[0],
                                                     frequencies.size(), sampleRate.load());
            std::transform(gains.begin(), gains.end(), singleMagnitudes.begin(), gains.begin(),
                           std::multiplies<FloatType>());
        }
        std::transform(gains.begin(), gains.end(), dBs.begin(),
                       [](auto &c) { return juce::Decibels::gainToDecibels(c, -FloatType(240)); });
        if (filterType.load() == FilterType::notch && order.load() >= 2) {
            auto freqIdx = static_cast<size_t>(std::floor(std::log(freq.load() / 10) / std::log(2200) *
                                                          static_cast<double>(frequencies.size())));
            if (freqIdx < frequencies.size()) {
                const auto dBl = dBs[freqIdx];
                dBs[freqIdx] = -90;
                freqIdx += 1;
                if (freqIdx < frequencies.size()) {
                    dBs[freqIdx] = -90 + dBl - dBs[freqIdx];
                }
            }
        }
    }

    template<typename FloatType>
    FloatType Filter<FloatType>::getDB(FloatType f) {
        double g{FloatType(1)};
        juce::dsp::IIR::Coefficients<FloatType> dummyCoeff;
        farbot::RealtimeObject<
            std::array<coeff33, 16>,
            farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
            farbot::ThreadType::nonRealtime> rrcentCoeffs(recentCoeffs);
        for (size_t i = 0; i < filterNum.load(); i++) {
            dummyCoeff = {
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[0]),
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[1]),
                static_cast<FloatType>(std::get<1>((*rrcentCoeffs)[i])[2]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[0]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[1]),
                static_cast<FloatType>(std::get<0>((*rrcentCoeffs)[i])[2])
            };
            g *= dummyCoeff.getMagnitudeForFrequency(static_cast<double>(f), sampleRate.load());
        }
        return juce::Decibels::gainToDecibels(static_cast<FloatType>(g), FloatType(-240));
    }

    template
    class Filter<float>;

    template
    class Filter<double>;
}
