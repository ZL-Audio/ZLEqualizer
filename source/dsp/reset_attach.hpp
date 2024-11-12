// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_RESET_ATTACH_HPP
#define ZLEqualizer_RESET_ATTACH_HPP

#include "controller.hpp"
#include "../state/state_definitions.hpp"

namespace zlDSP {
    template<typename FloatType>
    class ResetAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ResetAttach(juce::AudioProcessor &processor,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             Controller<FloatType> &controller);

        ~ResetAttach() override;

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef, &parameterNARef;
        Controller<FloatType> &controllerRef;

        constexpr static std::array resetIDs{
            zlDSP::solo::ID, zlDSP::sideSolo::ID,
            zlDSP::dynamicON::ID, zlDSP::dynamicLearn::ID,
            zlDSP::threshold::ID, zlDSP::kneeW::ID, zlDSP::attack::ID, zlDSP::release::ID,
            zlDSP::bypass::ID, zlDSP::fType::ID, zlDSP::slope::ID, zlDSP::lrType::ID
        };

        inline const static std::array resetDefaultVs{
            zlDSP::solo::convertTo01(zlDSP::solo::defaultV),
            zlDSP::sideSolo::convertTo01(zlDSP::sideSolo::defaultV),
            zlDSP::dynamicON::convertTo01(zlDSP::dynamicON::defaultV),
            zlDSP::dynamicLearn::convertTo01(zlDSP::dynamicLearn::defaultV),
            zlDSP::threshold::convertTo01(zlDSP::threshold::defaultV),
            zlDSP::kneeW::convertTo01(zlDSP::kneeW::defaultV),
            zlDSP::attack::convertTo01(zlDSP::attack::defaultV),
            zlDSP::release::convertTo01(zlDSP::release::defaultV),
            zlDSP::bypass::convertTo01(zlDSP::bypass::defaultV),
            zlDSP::fType::convertTo01(zlDSP::fType::defaultI),
            zlDSP::slope::convertTo01(zlDSP::slope::defaultI),
            zlDSP::lrType::convertTo01(zlDSP::lrType::defaultI),
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlDSP

#endif //ZLEqualizer_RESET_ATTACH_HPP
