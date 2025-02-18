// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_label.hpp"

namespace zlPanel {
    MatchLabel::MatchLabel(zlInterface::UIBase &base)
        : uiBase(base), labelLAF(base) {
        runningLabel.setText("Running", juce::dontSendNotification);
        runningLabel.setJustificationType(juce::Justification::centred);
        labelLAF.setFontScale(5.f);
        runningLabel.setLookAndFeel(&labelLAF);
        addAndMakeVisible(runningLabel);
        setBufferedToImage(true);
    }

    void MatchLabel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getColourByIdx(zlInterface::backgroundColour).withAlpha(.75f));
    }

    void MatchLabel::resized() {
        const auto bound = getLocalBounds().toFloat();
        runningLabel.setBounds(bound.withSizeKeepingCentre(
            bound.getWidth() * .5f, uiBase.getFontSize() * 5.f).toNearestInt());
    }
} // zlPanel
