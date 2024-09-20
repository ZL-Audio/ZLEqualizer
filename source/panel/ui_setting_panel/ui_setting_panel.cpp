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
    UISettingPanel::UISettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p), uiBase(base),
          internelPanel(p, uiBase),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          closeDrawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          resetDrawable(
              juce::Drawable::createFromImageData(BinaryData::loopleftline_svg, BinaryData::loopleftline_svgSize)),
          saveButton(saveDrawable.get(), uiBase),
          closeButton(closeDrawable.get(), uiBase),
          resetButton(resetDrawable.get(), uiBase),
          labelLAF(uiBase) {
        juce::ignoreUnused(pRef);
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

        labelLAF.setFontScale(1.125f);
        labelLAF.setAlpha(.5f);
        labelLAF.setJustification(juce::Justification::bottomLeft);
        versionLabel.setText(
            juce::String(ZLEQUALIZER_CURRENT_VERSION) + " " + juce::String(ZLEQUALIZER_CURRENT_HASH),
            juce::dontSendNotification);
        versionLabel.setLookAndFeel(&labelLAF);
        addAndMakeVisible(versionLabel);
    }

    UISettingPanel::~UISettingPanel() {
        versionLabel.setLookAndFeel(nullptr);
    }

    void UISettingPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight());
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.5f});
    }

    void UISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight());
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});

        internelPanel.setBounds(0, 0,
                                juce::roundToInt(bound.getWidth()),
                                juce::roundToInt(bound.getHeight() * 1.55f));
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

        bound = getLocalBounds().toFloat();
        bound = bound.removeFromBottom(2.f * uiBase.getFontSize());
        bound = bound.removeFromLeft(bound.getWidth() * .125f);
        bound.removeFromLeft(uiBase.getFontSize() * .25f);
        bound.removeFromBottom(uiBase.getFontSize() * .0625f);
        versionLabel.setBounds(bound.toNearestInt());
    }

    void UISettingPanel::loadSetting() {
        internelPanel.loadSetting();
    }
} // zlPanel
