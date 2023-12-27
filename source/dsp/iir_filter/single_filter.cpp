// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_filter.h"

namespace zlIIR {
    template<typename FloatType>
    void Filter<FloatType>::reset() {
        for (auto &f: filters) {
            f.reset();
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        processSpec = spec;
        for (auto &f: filters) {
            f.prepare(spec);
        }
        setOrder(order.load());
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
        const juce::ScopedReadLock scopedLock(paraUpdateLock);
        for (auto &f: filters) {
            f.process(context);
        }
    }

    template<typename FloatType>
    void Filter<FloatType>::setFreq(FloatType x) {
        freq.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setGain(FloatType x) {
        gain.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setQ(FloatType x) {
        q.store(static_cast<double>(x));
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setFilterType(zlIIR::FilterType x) {
        filterType.store(x);
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::setOrder(size_t x) {
        order.store(x);
        updateParas();
    }

    template<typename FloatType>
    void Filter<FloatType>::updateParas() {
        auto coeff = DesignFilter::getCoeff(filterType.load(),
                                            freq.load(), processSpec.sampleRate,
                                            gain.load(), q.load(), order.load());
        const juce::ScopedWriteLock scopedLock(paraUpdateLock);
        if (coeff.size() != filters.size()) {
            filters.clear();
            filters.resize(coeff.size());
            for (size_t i = 0; i < coeff.size(); i++) {
                filters[i].prepare(processSpec);
            }
        }
        for (size_t i = 0; i < coeff.size(); i++) {
            auto [a, b] = coeff[i];
            std::array<FloatType, 6> finalCoeff{};
            for (size_t j = 0; j < 6; ++j) {
                finalCoeff[j] = static_cast<FloatType>(j < 3 ? b[j] : a[j - 3]);
            }
            *filters[i].state = finalCoeff;
        }
        updateDBs();
    }

    template<typename FloatType>
    void Filter<FloatType>::addDBs(std::array<FloatType, frequencies.size()> &x, FloatType scale) {
        const juce::ScopedReadLock scopedLock(magLock);
        std::transform(x.begin(), x.end(), dBs.begin(), x.begin(),
                       [&scale](auto &c1, auto &c2) { return c1 + c2 * scale; });
    }

    template<typename FloatType>
    void Filter<FloatType>::addGains(std::array<FloatType, frequencies.size()> &x, FloatType scale) {
        const juce::ScopedReadLock scopedLock(magLock);
        std::transform(x.begin(), x.end(), gains.begin(), x.begin(),
                       [&scale](auto &c1, auto &c2) { return c1 + c2 * scale; });
    }

    template<typename FloatType>
    void Filter<FloatType>::updateDBs() {
        const juce::ScopedWriteLock scopedLock(magLock);
        gains.fill(FloatType(1));
        std::array<double, frequencies.size()> singleMagnitudes{};
        const juce::ScopedReadLock scopedLock2(paraUpdateLock);
        for (size_t i = 0; i < filters.size(); i++) {
            filters[i].state->getMagnitudeForFrequencyArray(&frequencies[0], &singleMagnitudes[0],
                                                            frequencies.size(), processSpec.sampleRate);
            std::transform(gains.begin(), gains.end(), singleMagnitudes.begin(), gains.begin(), std::multiplies<FloatType>());
        }
        std::transform(gains.begin(), gains.end(), dBs.begin(), [](auto &c) { return juce::Decibels::gainToDecibels(c); });
    }

    template
    class Filter<float>;

    template
    class Filter<double>;
}