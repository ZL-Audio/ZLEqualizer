// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlGain {
    template<typename FloatType>
    class OriginGain {
    public:
        OriginGain() noexcept = default;

        //==============================================================================
        /** Applies a new gain as a linear value. */
        void setGainLinear(FloatType newGain) noexcept { gain.setTargetValue(newGain); }

        /** Applies a new gain as a decibel value. */
        void setGainDecibels(FloatType newGainDecibels) noexcept {
            setGainLinear(juce::Decibels::decibelsToGain<FloatType>(newGainDecibels, (-240)));
        }

        /** Returns the target gain as a linear value. */
        FloatType getTargetGainLinear() const noexcept { return gain.getTargetValue(); }

        /** Returns the target gain in decibels. */
        FloatType getTargetGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getTargetGainLinear(), FloatType(-240));
        }

        /** Returns the current gain as a linear value. */
        FloatType getCurrentGainLinear() const noexcept { return gain.getCurrentValue(); }

        /** Returns the current gain in decibels. */
        FloatType getCurrentGainDecibels() const noexcept {
            return juce::Decibels::gainToDecibels<FloatType>(getCurrentGainLinear(), FloatType(-240));
        }

        /** Sets the length of the ramp used for smoothing gain changes. */
        void setRampDurationSeconds(const double newDurationSeconds) noexcept {
            if (!juce::approximatelyEqual(rampDurationSeconds, newDurationSeconds)) {
                rampDurationSeconds = newDurationSeconds;
                reset();
            }
        }

        /** Returns the ramp duration in seconds. */
        double getRampDurationSeconds() const noexcept { return rampDurationSeconds; }

        /** Returns true if the current value is currently being interpolated. */
        bool isSmoothing() const noexcept { return gain.isSmoothing(); }

        //==============================================================================
        /** Called before processing starts. */
        void prepare(const juce::dsp::ProcessSpec &spec) noexcept {
            sampleRate = spec.sampleRate;
            reset();
        }

        /** Resets the internal state of the gain */
        void reset() noexcept {
            if (sampleRate > 0)
                gain.reset(sampleRate, rampDurationSeconds);
        }

        //==============================================================================
        /** Returns the result of processing a single sample. */
        template<typename SampleType>
        SampleType JUCE_VECTOR_CALLTYPE processSample(SampleType s) noexcept {
            return s * gain.getNextValue();
        }

        /** Processes the input and output buffers supplied in the processing context. */
        template<typename ProcessContext>
        void process(const ProcessContext &context) noexcept {
            auto &&inBlock = context.getInputBlock();
            auto &&outBlock = context.getOutputBlock();

            jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
            jassert(inBlock.getNumSamples() == outBlock.getNumSamples());

            auto len = inBlock.getNumSamples();
            auto numChannels = inBlock.getNumChannels();

            if (context.isBypassed) {
                gain.skip(static_cast<int>(len));

                if (context.usesSeparateInputAndOutputBlocks())
                    outBlock.copyFrom(inBlock);

                return;
            }

            if (numChannels == 1) {
                auto *src = inBlock.getChannelPointer(0);
                auto *dst = outBlock.getChannelPointer(0);

                for (size_t i = 0; i < len; ++i)
                    dst[i] = src[i] * gain.getNextValue();
            } else {
                JUCE_BEGIN_IGNORE_WARNINGS_MSVC(6255 6386)
                auto *gains = static_cast<FloatType *>(alloca(sizeof (FloatType) * len));

                for (size_t i = 0; i < len; ++i)
                    gains[i] = gain.getNextValue();
                JUCE_END_IGNORE_WARNINGS_MSVC

                for (size_t chan = 0; chan < numChannels; ++chan)
                    juce::FloatVectorOperations::multiply(outBlock.getChannelPointer(chan),
                                                          inBlock.getChannelPointer(chan),
                                                          gains, static_cast<int>(len));
            }
        }

    private:
        juce::SmoothedValue<FloatType> gain;
        double sampleRate = 0, rampDurationSeconds = 0;
    };
}
