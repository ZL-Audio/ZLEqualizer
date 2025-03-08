// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_panel.hpp"
#include "BinaryData.h"

namespace zlPanel {
    UISettingPanel::UISettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p), uiBase(base),
          colourPanel(p, base),
          controlPanel(p, base),
          otherPanel(p, base),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          closeDrawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          resetDrawable(
              juce::Drawable::createFromImageData(BinaryData::loopleftline_svg, BinaryData::loopleftline_svgSize)),
          saveButton(uiBase, saveDrawable.get()),
          closeButton(uiBase, closeDrawable.get()),
          resetButton(uiBase, resetDrawable.get()),
          panelNameLAF(uiBase),
          labelLAF(uiBase) {
        juce::ignoreUnused(pRef);
        setOpaque(true);
        uiBase.setProperty(zlInterface::settingIdx::uiSettingPanelShow, false);
        addAndMakeVisible(saveButton);
        addAndMakeVisible(closeButton);
        addAndMakeVisible(resetButton);
        viewPort.setScrollBarsShown(true, false,
                                    true, false);
        changeDisplayPanel();
        addAndMakeVisible(viewPort);
        saveButton.getButton().onClick = [this]() {
            switch (currentPanelIdx) {
                case colourP: {
                    colourPanel.saveSetting();
                    break;
                }
                case controlP: {
                    controlPanel.saveSetting();
                    break;
                }
                case otherP: {
                    otherPanel.saveSetting();
                    break;
                }
            }
        };
        resetButton.getButton().onClick = [this]() {
            switch (currentPanelIdx) {
                case colourP: {
                    colourPanel.resetSetting();
                    break;
                }
                case controlP: {
                    controlPanel.resetSetting();
                    break;
                }
                case otherP: {
                    otherPanel.resetSetting();
                    break;
                }
            }
        };
        closeButton.getButton().onClick = [this]() {
            setVisible(false);
        };

        panelNameLAF.setFontScale(1.5f);
        panelLabels[0].setText("Colour", juce::dontSendNotification);
        panelLabels[1].setText("Control", juce::dontSendNotification);
        panelLabels[2].setText("Other", juce::dontSendNotification);
        for (auto &pL: panelLabels) {
            pL.setInterceptsMouseClicks(true, false);
            pL.addMouseListener(this, false);
            pL.setJustificationType(juce::Justification::centred);
            pL.setLookAndFeel(&panelNameLAF);
            addAndMakeVisible(pL);
        }

        labelLAF.setFontScale(1.125f);
        labelLAF.setAlpha(.5f);
        versionLabel.setText(
            juce::String(ZLEQUALIZER_CURRENT_VERSION) + " " + juce::String(ZLEQUALIZER_CURRENT_HASH),
            juce::dontSendNotification);
        versionLabel.setJustificationType(juce::Justification::bottomLeft);
        versionLabel.setLookAndFeel(&labelLAF);
        addAndMakeVisible(versionLabel);
    }

    UISettingPanel::~UISettingPanel() {
        for (auto &pL: panelLabels) {
            pL.setLookAndFeel(nullptr);
        }
        versionLabel.setLookAndFeel(nullptr);
    }

    void UISettingPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * 1.25f);
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.5f});
    }

    void UISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight());
        {
            auto labelBound = bound.removeFromTop(uiBase.getFontSize() * 3.f);
            const auto labelWidth = labelBound.getWidth() / static_cast<float>(panelLabels.size());
            for (auto & panelLabel : panelLabels) {
                panelLabel.setBounds(labelBound.removeFromLeft(labelWidth).toNearestInt());
            }
        }

        colourPanel.setBounds(0, 0,
                              juce::roundToInt(bound.getWidth()),
                              juce::roundToInt(uiBase.getFontSize() * (ColourSettingPanel::heightP + 1.f)));
        controlPanel.setBounds(0, 0,
                               juce::roundToInt(bound.getWidth()),
                               juce::roundToInt(uiBase.getFontSize() * (ControlSettingPanel::heightP + 1.f)));
        otherPanel.setBounds(0, 0,
                             juce::roundToInt(bound.getWidth()),
                             juce::roundToInt(uiBase.getFontSize() * (OtherUISettingPanel::heightP + 1.f)));

        viewPort.setBounds(bound.removeFromTop(bound.getHeight() * .9125f).toNearestInt());
        const auto leftBound = bound.removeFromLeft(
            bound.getWidth() * .3333333f).withSizeKeepingCentre(
            uiBase.getFontSize() * 2.f, uiBase.getFontSize() * 2.f);
        const auto centerBound = bound.removeFromLeft(
            bound.getWidth() * .5f).withSizeKeepingCentre(
            uiBase.getFontSize() * 2.f, uiBase.getFontSize() * 2.f);
        const auto rightBound = bound.withSizeKeepingCentre(
            uiBase.getFontSize() * 2.f, uiBase.getFontSize() * 2.f);
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
        colourPanel.loadSetting();
        controlPanel.loadSetting();
        otherPanel.loadSetting();
    }

    void UISettingPanel::mouseDown(const juce::MouseEvent &event) {
        for (size_t i = 0; i < panelLabels.size(); ++i) {
            if (event.originalComponent == &panelLabels[i]) {
                currentPanelIdx = static_cast<panelIdx>(i);
                changeDisplayPanel();
                return;
            }
        }
    }

    void UISettingPanel::changeDisplayPanel() {
        switch (currentPanelIdx) {
            case colourP: {
                viewPort.setViewedComponent(&colourPanel, false);
                break;
            }
            case controlP: {
                viewPort.setViewedComponent(&controlPanel, false);
                break;
            }
            case otherP: {
                viewPort.setViewedComponent(&otherPanel, false);
                break;
            }
        }
    }

    void UISettingPanel::visibilityChanged() {
        uiBase.setProperty(zlInterface::settingIdx::uiSettingPanelShow, isVisible());
    }

    void UISettingPanel::setRendererList(const juce::StringArray &rendererList) {
        otherPanel.setRendererList(rendererList);
    }
} // zlPanel
