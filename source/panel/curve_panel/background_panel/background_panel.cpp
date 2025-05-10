// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "background_panel.hpp"

namespace zlpanel {
    BackgroundPanel::BackgroundPanel(juce::AudioProcessorValueTreeState &parameters,
                                 juce::AudioProcessorValueTreeState &parameters_NA,
                                 zlgui::UIBase &base)
        : ui_base_(base),
          grid_panel_(base) {
        juce::ignoreUnused(parameters, parameters_NA);
        setInterceptsMouseClicks(false, true);
        setOpaque(true);
        addAndMakeVisible(grid_panel_);
        setBufferedToImage(true);
    }

    BackgroundPanel::~BackgroundPanel() = default;

    void BackgroundPanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getBackgroundColor());
    }

    void BackgroundPanel::resized() {
        grid_panel_.setBounds(getLocalBounds());
    }
}
