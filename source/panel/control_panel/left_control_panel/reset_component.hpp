// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_RESET_COMPONENT_HPP
#define ZLEqualizer_RESET_COMPONENT_HPP

#include "BinaryData.h"

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class ResetComponent final : public juce::Component {
    public:
        explicit ResetComponent(juce::AudioProcessorValueTreeState &parameters,
                                juce::AudioProcessorValueTreeState &parametersNA,
                                zlInterface::UIBase &base);

        ~ResetComponent() override;

        void resized() override;

        void attachGroup(size_t idx);

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        const std::unique_ptr<juce::Drawable> drawable;
        zlInterface::ClickButton button;
        std::atomic<size_t> bandIdx;
    };
} // zlPanel

#endif //ZLEqualizer_RESET_COMPONENT_HPP
