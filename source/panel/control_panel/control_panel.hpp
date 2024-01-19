// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONTROL_PANEL_HPP
#define ZLEqualizer_CONTROL_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "../../gui/gui.hpp"
#include "left_control_panel/left_control_panel.hpp"
#include "right_control_panel/right_control_panel.hpp"

namespace zlPanel {
    class ControlPanel final : public juce::Component,
                               private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ControlPanel(juce::AudioProcessorValueTreeState &parameters,
                              juce::AudioProcessorValueTreeState &parametersNA,
                              zlInterface::UIBase &base);

        ~ControlPanel() override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;
        LeftControlPanel leftControlPanel;
        RightControlPanel rightControlPanel;

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlPanel

#endif //ZLEqualizer_CONTROL_PANEL_HPP
