// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SOLO_ATTACH_HPP
#define ZLEqualizer_SOLO_ATTACH_HPP

#include "controller.hpp"

namespace zlDSP {
    template<typename FloatType>
    class SoloAttach final : private juce::AudioProcessorValueTreeState::Listener {
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

        std::atomic<size_t> soloIdx{0};
        std::atomic<bool> soloIsSide{false};

        constexpr static std::array initIDs{
            solo::ID, sideSolo::ID
        };

        constexpr static std::array defaultVs{
            static_cast<float>(solo::defaultV),
            static_cast<float>(sideSolo::defaultV)
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void initDefaultValues();
    };
}

#endif //ZLEqualizer_SOLO_ATTACH_HPP
