// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "setting_panel.hpp"
#include "../../state/state.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    SettingPanel::SettingPanel(PluginProcessor &p, zlInterface::UIBase &base,
                               juce::String label, zlInterface::boxIdx idx)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          mIdx(idx) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText(label, juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        name.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(name);
        setBufferedToImage(true);
    }

    SettingPanel::~SettingPanel() {
        stopTimer();
    }

    void SettingPanel::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
    }

    void SettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer();
        if (uiBase.getBoxProperty(mIdx)) {
            uiBase.setBoxProperty(mIdx, false);
        } else {
            uiBase.openOneBox(mIdx);
        }
    }

    void SettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        startTimer(100);
    }

    void SettingPanel::mouseExit(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer();
    }

    void SettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void SettingPanel::timerCallback() {
        uiBase.openOneBox(mIdx);
    }
} // zlPanel
