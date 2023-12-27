// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_FILTERS_ATTACH_H
#define ZLEQUALIZER_FILTERS_ATTACH_H

#include "controller.h"

namespace zlDSP {
    template<typename FloatType>
    class FiltersAttach : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FiltersAttach(juce::AudioProcessor &processor,
                               juce::AudioProcessorValueTreeState &parameters,
                               Controller<FloatType> &controller);

        ~FiltersAttach() override;

        void addListeners();

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef;
        Controller<FloatType> &controllerRef;
        std::array<zlDynamicFilter::IIRFilter<FloatType>, bandNUM> &filtersRef;

        constexpr static std::array IDs{bypass::ID, fType::ID, slope::ID, freq::ID, gain::ID, Q::ID,
                                        lrType::ID, dynamicON::ID, dynamicBypass::ID,
                                        targetGain::ID, targetQ::ID, threshold::ID, ratio::ID,
                                        sideFreq::ID, attack::ID, release::ID, sideQ::ID};

        constexpr static std::array dynamicInitIDs{dynamicBypass::ID, targetGain::ID, targetQ::ID, sideFreq::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
}

#endif //ZLEQUALIZER_FILTERS_ATTACH_H
