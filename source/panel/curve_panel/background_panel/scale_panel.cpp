// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"

#include "../../panel_definitons.hpp"

namespace zlPanel {
    ScalePanel::ScalePanel(juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : parametersNARef(parametersNA), uiBase(base),
          scaleBox("", zlState::maximumDB::choices, base) {
        juce::ignoreUnused(uiBase);
        scaleBox.getLAF().setFontScale(zlInterface::FontLarge);
        attach({&scaleBox.getBox()}, {zlState::maximumDB::ID}, parametersNARef, boxAttachments);
        addAndMakeVisible(scaleBox);
    }

    ScalePanel::~ScalePanel() = default;

    void ScalePanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());

        g.setFont(uiBase.getFontSize() * zlInterface::FontLarge);
        for (auto &d:scaleDBs) {
            const auto y = d * bound.getHeight() + bound.getY() - uiBase.getFontSize() * .75f;
            const auto lTextBound = juce::Rectangle<float>(bound.getX(), y, bound.getWidth() * .4f, uiBase.getFontSize() * 1.5f);
            const auto leftDB = static_cast<int>(-2 * d * maximumDB.load() + maximumDB.load());
            g.setColour(uiBase.getTextColor());
            g.drawText(juce::String(leftDB), lTextBound, juce::Justification::centredRight);
            const auto rTextBound = juce::Rectangle<float>(bound.getCentreX(), y, bound.getWidth() * .4f, uiBase.getFontSize() * 1.5f);
            const auto reftDB = static_cast<int>(maximumFFTDB.load() * d);
            g.setColour(uiBase.getTextInactiveColor());
            g.drawText(juce::String(reftDB), rTextBound, juce::Justification::centredRight);
        }
    }

    void ScalePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        auto boxBound = juce::Rectangle<float>(uiBase.getFontSize() * 4.f, uiBase.getFontSize() * 1.5f);
        boxBound = boxBound.withCentre({bound.getCentreX(), bound.getY()});
        boxBound.removeFromRight(uiBase.getFontSize());
        scaleBox.setBounds(boxBound.toNearestInt());
    }

    void ScalePanel::handleAsyncUpdate() {
        repaint();
    }
} // zlPanel
