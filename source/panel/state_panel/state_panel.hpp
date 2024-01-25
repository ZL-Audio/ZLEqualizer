// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_STATE_PANEL_HPP
#define ZLEqualizer_STATE_PANEL_HPP

#include "logo_panel.hpp"

namespace zlPanel {

    class StatePanel final : public juce::Component {
    public:
        explicit StatePanel(juce::AudioProcessorValueTreeState &parameters,
                           juce::AudioProcessorValueTreeState &parametersNA,
                           juce::AudioProcessorValueTreeState &state,
                           zlInterface::UIBase &base);

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        juce::AudioProcessorValueTreeState &parametersNARef;
        juce::AudioProcessorValueTreeState &stateRef;
        zlInterface::UIBase &uiBase;

        LogoPanel logoPanel;
    };

} // zlPanel

#endif //ZLEqualizer_STATE_PANEL_HPP
