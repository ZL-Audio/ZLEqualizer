// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_panel.hpp"
#include "BinaryData.h"

namespace zlPanel {
    UISettingPanel::UISettingPanel(zlInterface::UIBase &base)
        : uiBase(base),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          closeDrawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          resetDrawable(juce::Drawable::createFromImageData(BinaryData::loopleftline_svg, BinaryData::loopleftline_svgSize)),
          saveButton(saveDrawable.get(), uiBase),
          closeButton(closeDrawable.get(), uiBase),
          resetButton(resetDrawable.get(), uiBase) {
        setOpaque(true);
        addAndMakeVisible(saveButton);
        addAndMakeVisible(closeButton);
        addAndMakeVisible(resetButton);
        viewPort.setScrollBarsShown(true, false,
                                    true, false);
        viewPort.setViewedComponent(&internelPanel, false);
        addAndMakeVisible(viewPort);
        saveButton.getButton().onClick = [this]() {
            internelPanel.saveSetting();
        };
        resetButton.getButton().onClick = [this]() {
            internelPanel.resetSetting();
        };
        closeButton.getButton().onClick = [this]() {
            setVisible(false);
        };
    }

    void UISettingPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
    }

    void UISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        internelPanel.setBounds(0, 0,
                                juce::roundToInt(bound.getWidth()),
                                juce::roundToInt(bound.getHeight()) * 2);
        viewPort.setBounds(bound.removeFromTop(bound.getHeight() * .9125f).toNearestInt());
        const auto leftBound = bound.removeFromLeft(
            bound.getWidth() * .3333333f).withSizeKeepingCentre(
            uiBase.getFontSize() * 1.25f, uiBase.getFontSize() * 1.25f);
        const auto centerBound = bound.removeFromLeft(
            bound.getWidth() * .5f).withSizeKeepingCentre(
            uiBase.getFontSize() * 1.25f, uiBase.getFontSize() * 1.25f);
        const auto rightBound = bound.withSizeKeepingCentre(
            uiBase.getFontSize() * 1.25f, uiBase.getFontSize() * 1.25f);
        saveButton.setBounds(leftBound.toNearestInt());
        resetButton.setBounds(centerBound.toNearestInt());
        closeButton.setBounds(rightBound.toNearestInt());
    }
} // zlPanel
