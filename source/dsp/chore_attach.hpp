// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CHORE_ATTACH_HPP
#define ZLEqualizer_CHORE_ATTACH_HPP

#include "controller.hpp"
#include "../state/state_definitions.hpp"

namespace zlDSP {
    template<typename FloatType>
    class ChoreAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ChoreAttach(juce::AudioProcessor &processor,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             Controller<FloatType> &controller);

        ~ChoreAttach() override;

        void addListeners();

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef, &parameterNARef;
        Controller<FloatType> &controllerRef;
        std::atomic<float> decaySpeed;
        std::array<std::atomic<int>, 3> isFFTON{1, 1, 0};

        constexpr static std::array IDs{
            sideChain::ID, dynLookahead::ID,
            dynRMS::ID, dynSmooth::ID,
            effectON::ID, phaseFlip::ID, staticAutoGain::ID, autoGain::ID,
            scale::ID, outputGain::ID,
            filterStructure::ID, dynLink::ID, dynHQ::ID, zeroLatency::ID
        };
        constexpr static std::array defaultVs{
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
            static_cast<float>(dynLink::defaultI),
            static_cast<float>(dynHQ::defaultI),
            static_cast<float>(zeroLatency::defaultI)
        };

        constexpr static std::array NAIDs{
            zlState::fftPreON::ID, zlState::fftPostON::ID, zlState::fftSideON::ID,
            zlState::ffTSpeed::ID, zlState::ffTTilt::ID,
            zlState::conflictON::ID,
            zlState::conflictStrength::ID,
            zlState::conflictScale::ID
        };

        constexpr static std::array defaultNAVs {
            static_cast<float>(zlState::fftPreON::defaultI),
            static_cast<float>(zlState::fftPostON::defaultI),
            static_cast<float>(zlState::fftSideON::defaultI),
            static_cast<float>(zlState::ffTSpeed::defaultI),
            static_cast<float>(zlState::ffTTilt::defaultI),
            static_cast<float>(zlState::conflictON::defaultI),
            static_cast<float>(zlState::conflictStrength::defaultV),
            static_cast<float>(zlState::conflictScale::defaultV)
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void initDefaultValues();
    };
} // zlDSP

#endif //ZLEqualizer_CHORE_ATTACH_HPP
