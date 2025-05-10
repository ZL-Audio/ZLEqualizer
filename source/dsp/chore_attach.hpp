// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "controller.hpp"
#include "../state/state_definitions.hpp"

namespace zlp {
    template<typename FloatType>
    class ChoreAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ChoreAttach(juce::AudioProcessor &processor,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA,
                             Controller<FloatType> &controller);

        ~ChoreAttach() override;

        void addListeners();

    private:
        juce::AudioProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        Controller<FloatType> &controller_ref_;
        std::atomic<float> decay_speed_;
        std::array<std::atomic<int>, 3> is_fft_on_{1, 1, 0};

        constexpr static std::array kIDs{
            sideChain::ID, dynLookahead::ID,
            dynRMS::ID, dynSmooth::ID,
            effectON::ID, phaseFlip::ID, staticAutoGain::ID, autoGain::ID,
            scale::ID, outputGain::ID,
            filterStructure::ID, dynHQ::ID, zeroLatency::ID,
            loudnessMatcherON::ID
        };
        constexpr static std::array kDefaultVs{
            static_cast<float>(sideChain::defaultV),
            static_cast<float>(dynLookahead::defaultV),
            static_cast<float>(dynRMS::defaultV),
            static_cast<float>(dynSmooth::defaultV),
            static_cast<float>(effectON::defaultI),
            static_cast<float>(phaseFlip::defaultI),
            static_cast<float>(staticAutoGain::defaultI),
            static_cast<float>(autoGain::defaultI),
            static_cast<float>(scale::defaultV),
            static_cast<float>(outputGain::defaultV),
            static_cast<float>(filterStructure::defaultI),
            static_cast<float>(dynHQ::defaultI),
            static_cast<float>(zeroLatency::defaultI),
            static_cast<float>(loudnessMatcherON::defaultV)
        };

        constexpr static std::array kNAIDs{
            zlstate::fftPreON::ID, zlstate::fftPostON::ID, zlstate::fftSideON::ID,
            zlstate::ffTSpeed::ID, zlstate::ffTTilt::ID,
            zlstate::conflictON::ID,
            zlstate::conflictStrength::ID,
            zlstate::conflictScale::ID
        };

        constexpr static std::array kDefaultNAVs{
            static_cast<float>(zlstate::fftPreON::defaultI),
            static_cast<float>(zlstate::fftPostON::defaultI),
            static_cast<float>(zlstate::fftSideON::defaultI),
            static_cast<float>(zlstate::ffTSpeed::defaultI),
            static_cast<float>(zlstate::ffTTilt::defaultI),
            static_cast<float>(zlstate::conflictON::defaultI),
            static_cast<float>(zlstate::conflictStrength::defaultV),
            static_cast<float>(zlstate::conflictScale::defaultV)
        };

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void initDefaultValues();
    };
} // zldsp
