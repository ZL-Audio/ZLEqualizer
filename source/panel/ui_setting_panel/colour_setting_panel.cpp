// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_setting_panel.hpp"

namespace zlPanel {
    static juce::Colour getIntColour(const int r, const int g, const int b, float alpha) {
        return {
            static_cast<juce::uint8>(r),
            static_cast<juce::uint8>(g),
            static_cast<juce::uint8>(b),
            alpha
        };
    }

    ColourSettingPanel::ColourSettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p), uiBase(base), nameLAF(base),
          textSelector(base, *this, false),
          backgroundSelector(base, *this, false),
          shadowSelector(base, *this, false),
          glowSelector(base, *this, false),
          preSelector(base, *this),
          postSelector(base, *this),
          sideSelector(base, *this),
          gridSelector(base, *this),
          tagSelector(base, *this),
          gainSelector(base, *this) {
        juce::ignoreUnused(pRef);
        nameLAF.setJustification(juce::Justification::centredRight);
        nameLAF.setFontScale(zlInterface::FontHuge);
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setText(selectorNames[i], juce::dontSendNotification);
            selectorLabels[i].setLookAndFeel(&nameLAF);
            addAndMakeVisible(selectorLabels[i]);
            addAndMakeVisible(selectors[i]);
        }
    }

    ColourSettingPanel::~ColourSettingPanel() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setLookAndFeel(nullptr);
        }
    }

    void ColourSettingPanel::loadSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectors[i]->setColour(uiBase.getColourByIdx(static_cast<zlInterface::colourIdx>(i)));
        }
    }

    void ColourSettingPanel::saveSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            uiBase.setColourByIdx(static_cast<zlInterface::colourIdx>(i), selectors[i]->getColour());
        }
        uiBase.saveToAPVTS();
    }

    void ColourSettingPanel::resetSetting() {
        textSelector.setColour(getIntColour(247, 246, 244, 1.f));
        backgroundSelector.setColour(getIntColour((255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, 1.f));
        shadowSelector.setColour(getIntColour(0, 0, 0, 1.f));
        glowSelector.setColour(getIntColour(70, 66, 62, 1.f));
        preSelector.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .1f));
        postSelector.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .1f));
        sideSelector.setColour(getIntColour(252, 18, 197, .1f));
        gridSelector.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .25f));
        saveSetting();
    }

    void ColourSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < numSelectors; ++i) {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            selectorLabels[i].setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            selectors[i]->setBounds(localBound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        }
    }
} // zlPanel
