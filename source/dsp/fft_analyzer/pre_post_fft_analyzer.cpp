// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "pre_post_fft_analyzer.hpp"

namespace zlFFT {
    template<typename FloatType>
    PrePostFFTAnalyzer<FloatType>::PrePostFFTAnalyzer() = default;

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {

        preFFT.prepare(spec);
        postFFT.prepare(spec);
        preBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        preDelay.prepare(spec);
        preDelay.setMaximumDelayInSamples(static_cast<int>(spec.sampleRate / 100));
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPreFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        preBuffer.makeCopyOf(buffer, true);
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::pushPostFFTBuffer(juce::AudioBuffer<FloatType> &buffer) {
        if (isON.load() && !preFFT.getIsAudioReady() && !postFFT.getIsAudioReady()) {
            // logger.logMessage(juce::String(preBuffer.getNumChannels()));
            juce::dsp::AudioBlock<FloatType> preBLock(preBuffer);
            preDelay.process(juce::dsp::ProcessContextReplacing<FloatType>(preBLock));
            preFFT.process(preBuffer);
            postFFT.process(buffer);
        }
    }

    template<typename FloatType>
    void PrePostFFTAnalyzer<FloatType>::setPreDelay(size_t numSamples) {
        preDelay.setDelay(static_cast<float>(numSamples));
    }

    template
    class PrePostFFTAnalyzer<float>;

    template
    class PrePostFFTAnalyzer<double>;
} // zlFFT
