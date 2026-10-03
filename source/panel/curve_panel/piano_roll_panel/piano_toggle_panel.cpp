// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "piano_toggle_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    PianoTogglePanel::PianoTogglePanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        drawable_(juce::Drawable::createFromImageData(BinaryData::piano_svg, BinaryData::piano_svgSize)),
        button_(base, drawable_.get(), drawable_.get()),
        attachment_(button_.getButton(), p.parameters_NA_, zlstate::PPianoRollON::kID, updater_) {
        button_.getButton().onStateChange = [this]() {
            repaint();
        };
        addAndMakeVisible(button_);
        setInterceptsMouseClicks(false, true);
        setBufferedToImage(true);
        updater_.updateComponents();
    }

    void PianoTogglePanel::paint(juce::Graphics& g) {
        if (button_.getToggleState()) {
            const auto bound = getLocalBounds().toFloat();
            const auto colour = base_.getBackgroundColour();
            const juce::ColourGradient gradient(colour, 0.f, 0.f,
                                                colour.withAlpha(0.f), bound.getRight(), 0.f, false);
            g.setGradientFill(gradient);
            g.fillRect(bound);
        }
    }

    void PianoTogglePanel::resized() {
        const auto bound = getLocalBounds();
        button_.setBounds(bound.withWidth(bound.getHeight()));
        button_.getButton().setEdgeIndent(static_cast<int>(std::round(base_.getFontSize() * .2f)));
    }

    void PianoTogglePanel::repaintCallBack() {
        updater_.updateComponents();
    }
}
