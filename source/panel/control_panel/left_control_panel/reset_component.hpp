// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_RESET_COMPONENT_HPP
#define ZLEqualizer_RESET_COMPONENT_HPP

#include "BinaryData.h"

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class ResetComponent : public juce::Component,
                           private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ResetComponent(juce::AudioProcessorValueTreeState &parameters,
                                juce::AudioProcessorValueTreeState &parametersNA,
                                zlInterface::UIBase &base);

        ~ResetComponent() override;

        void resized() override;

        void attachGroup(size_t idx);

        void resetBand();

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        const std::unique_ptr<juce::Drawable> drawable;
        zlInterface::ClickButton button;
        std::atomic<size_t> bandIdx;

        constexpr static std::array resetIDs{
            zlDSP::dynamicON::ID, zlDSP::threshold::ID, zlDSP::kneeW::ID, zlDSP::attack::ID, zlDSP::release::ID,
            zlDSP::bypass::ID, zlDSP::fType::ID, zlDSP::slope::ID, zlDSP::freq::ID,
            zlDSP::gain::ID, zlDSP::Q::ID, zlDSP::lrType::ID
        };

        inline const static std::array resetDefaultVs{
            zlDSP::dynamicON::convertTo01(zlDSP::dynamicON::defaultV),
            zlDSP::threshold::convertTo01(zlDSP::threshold::defaultV),
            zlDSP::kneeW::convertTo01(zlDSP::kneeW::defaultV),
            zlDSP::attack::convertTo01(zlDSP::attack::defaultV),
            zlDSP::release::convertTo01(zlDSP::release::defaultV),
            zlDSP::bypass::convertTo01(zlDSP::bypass::defaultV),
            zlDSP::fType::convertTo01(zlDSP::fType::defaultI),
            zlDSP::slope::convertTo01(zlDSP::slope::defaultI),
            zlDSP::freq::convertTo01(zlDSP::freq::defaultV),
            zlDSP::gain::convertTo01(zlDSP::gain::defaultV),
            zlDSP::Q::convertTo01(zlDSP::Q::defaultV),
            zlDSP::lrType::convertTo01(zlDSP::lrType::defaultI)
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlPanel

#endif //ZLEqualizer_RESET_COMPONENT_HPP
