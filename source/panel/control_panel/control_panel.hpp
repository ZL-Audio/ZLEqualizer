// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONTROL_PANEL_HPP
#define ZLEqualizer_CONTROL_PANEL_HPP

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "left_control_panel/left_control_panel.hpp"
#include "right_control_panel/right_control_panel.hpp"

namespace zlPanel {
    class ControlPanel final : public juce::Component,
                               private juce::AudioProcessorValueTreeState::Listener,
                               private juce::AsyncUpdater {
    public:
        explicit ControlPanel(PluginProcessor &p,
                              zlInterface::UIBase &base);

        ~ControlPanel() override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<size_t> bandIdx{0};
        LeftControlPanel leftControlPanel;
        RightControlPanel rightControlPanel;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //ZLEqualizer_CONTROL_PANEL_HPP
