// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dynamic_setting_panel.hpp"

namespace zlPanel {
    DynamicSettingPanel::DynamicSettingPanel(PluginProcessor &p,
                                             zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Dynamic", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        name.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(name);
        setBufferedToImage(true);
    }

    DynamicSettingPanel::~DynamicSettingPanel() = default;

    void DynamicSettingPanel::paint(juce::Graphics &g) {
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

    void DynamicSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (uiBase.getBoxProperty(zlInterface::boxIdx::dynamicBox)) {
            uiBase.setBoxProperty(zlInterface::boxIdx::dynamicBox, false);
        } else {
            uiBase.openOneBox(zlInterface::boxIdx::dynamicBox);
        }
    }

    void DynamicSettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        uiBase.openOneBox(zlInterface::boxIdx::dynamicBox);
    }

    void DynamicSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }
} // zlPanel
