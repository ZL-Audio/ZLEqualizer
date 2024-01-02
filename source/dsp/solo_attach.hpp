// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SOLO_ATTACH_HPP
#define ZLEqualizer_SOLO_ATTACH_HPP

#include "controller.hpp"

namespace zlDSP {
    template<typename FloatType>
    class SoloAttach : private juce::AudioProcessorValueTreeState::Listener,
                       private juce::AsyncUpdater {
    public:
        explicit SoloAttach(juce::AudioProcessor &processor,
                            juce::AudioProcessorValueTreeState &parameters,
                            Controller<FloatType> &controller);

        ~SoloAttach() override;

        void addListeners();

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef;
        Controller<FloatType> &controllerRef;

        constexpr static std::array IDs{
            fType::ID,
            freq::ID, Q::ID, sideFreq::ID, sideQ::ID,
            solo::ID, sideSolo::ID
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
}

#endif //ZLEqualizer_SOLO_ATTACH_HPP
