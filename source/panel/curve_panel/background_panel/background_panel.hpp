// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_BACKGROUND_PANEL_HPP
#define ZLEqualizer_BACKGROUND_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../gui/gui.hpp"
#include "grid_panel.hpp"
#include "scale_panel.hpp"

namespace zlPanel {
    class BackgroundPanel final : public juce::Component {
    public:
        explicit BackgroundPanel(juce::AudioProcessorValueTreeState &parameters,
                                 juce::AudioProcessorValueTreeState &parametersNA,
                                 zlInterface::UIBase &base);

        ~BackgroundPanel() override;

        void resized() override;

        void setMaximumDB(const float x) { scalePanel.setMaximumDB(x); }

    private:
        zlInterface::UIBase &uiBase;
        GridPanel gridPanel;
        ScalePanel scalePanel;
    };
}

#endif //ZLEqualizer_BACKGROUND_PANEL_HPP
