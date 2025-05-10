// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_label.hpp"

namespace zlpanel {
    MatchLabel::MatchLabel(zlgui::UIBase &base)
        : ui_base_(base), label_laf_(base) {
        running_label_.setText("Running", juce::dontSendNotification);
        running_label_.setJustificationType(juce::Justification::centred);
        label_laf_.setFontScale(5.f);
        running_label_.setLookAndFeel(&label_laf_);
        addAndMakeVisible(running_label_);
        setBufferedToImage(true);
    }

    void MatchLabel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getColourByIdx(zlgui::kBackgroundColour).withAlpha(.75f));
    }

    void MatchLabel::resized() {
        const auto bound = getLocalBounds().toFloat();
        running_label_.setBounds(bound.withSizeKeepingCentre(
            bound.getWidth() * .5f, ui_base_.getFontSize() * 5.f).toNearestInt());
    }
} // zlpanel
