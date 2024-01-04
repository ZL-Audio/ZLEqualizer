// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_MAIN_PANEL_HPP
#define ZLEqualizer_MAIN_PANEL_HPP

#include "control_panel/control_panel.hpp"

namespace zlPanel {
    class MainPanel final : public juce::Component {
    public:
        MainPanel(juce::AudioProcessorValueTreeState &parameters,
                  juce::AudioProcessorValueTreeState &parametersNA);

        ~MainPanel() override = default;

        void paint(juce::Graphics &g) override;

        void resized() override;
    private:
        zlInterface::UIBase uiBase;
        ControlPanel controlPanel;
    };
}


#endif //ZLEqualizer_MAIN_PANEL_HPP
