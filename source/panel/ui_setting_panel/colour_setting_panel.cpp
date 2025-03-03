// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
          gainSelector(base, *this),
          sideLoudnessSelector(base, *this),
          cMap1Selector(base), cMap2Selector(base) {
        juce::ignoreUnused(pRef);
        if (!settingDirectory.isDirectory()) {
            settingDirectory.createDirectory();
        }
        nameLAF.setFontScale(zlInterface::FontHuge);
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setText(selectorNames[i], juce::dontSendNotification);
            selectorLabels[i].setJustificationType(juce::Justification::centredRight);
            selectorLabels[i].setLookAndFeel(&nameLAF);
            addAndMakeVisible(selectorLabels[i]);
            addAndMakeVisible(selectors[i]);
        }
        cMap1Label.setText("Colour Map 1", juce::dontSendNotification);
        cMap1Label.setJustificationType(juce::Justification::centredRight);
        cMap1Label.setLookAndFeel(&nameLAF);
        addAndMakeVisible(cMap1Label);
        addAndMakeVisible(cMap1Selector);
        cMap2Label.setText("Colour Map 2", juce::dontSendNotification);
        cMap2Label.setJustificationType(juce::Justification::centredRight);
        cMap2Label.setLookAndFeel(&nameLAF);
        addAndMakeVisible(cMap2Label);
        addAndMakeVisible(cMap2Selector);
        importLabel.setText("Import Colours", juce::dontSendNotification);
        importLabel.setJustificationType(juce::Justification::centred);
        importLabel.setLookAndFeel(&nameLAF);
        importLabel.addMouseListener(this, false);
        addAndMakeVisible(importLabel);
        exportLabel.setText("Export Colours", juce::dontSendNotification);
        exportLabel.setJustificationType(juce::Justification::centred);
        exportLabel.setLookAndFeel(&nameLAF);
        exportLabel.addMouseListener(this, false);
        addAndMakeVisible(exportLabel);
    }

    ColourSettingPanel::~ColourSettingPanel() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setLookAndFeel(nullptr);
        }
    }

    void ColourSettingPanel::loadSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectors[i]->setColour(uiBase.getColourByIdx(colourIdx[i]));
        }
        cMap1Selector.getBox().setSelectedId(static_cast<int>(uiBase.getCMap1Idx()) + 1);
        cMap2Selector.getBox().setSelectedId(static_cast<int>(uiBase.getCMap2Idx()) + 1);
    }

    void ColourSettingPanel::saveSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            uiBase.setColourByIdx(colourIdx[i], selectors[i]->getColour());
        }
        uiBase.setCMap1Idx(static_cast<size_t>(cMap1Selector.getBox().getSelectedId() - 1));
        uiBase.setCMap2Idx(static_cast<size_t>(cMap2Selector.getBox().getSelectedId() - 1));
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
        cMap1Selector.getBox().setSelectedId(zlState::colourMap1Idx::defaultI + 1);
        cMap2Selector.getBox().setSelectedId(zlState::colourMap2Idx::defaultI + 1);
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
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            cMap1Label.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            cMap1Selector.setBounds(localBound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            cMap2Label.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            cMap2Selector.setBounds(localBound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            importLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .45f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .10f);
            exportLabel.setBounds(localBound.toNearestInt());
        }
    }

    void ColourSettingPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent == &importLabel) {
            myChooser = std::make_unique<juce::FileChooser>(
                "Load the colour settings...", settingDirectory, "*.xml",
                true, false, nullptr);
            constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                              juce::FileBrowserComponent::canSelectFiles;
            myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
                if (chooser.getResults().size() <= 0) { return; }
                const juce::File settingFile(chooser.getResult());
                if (const auto xmlInput = juce::XmlDocument::parse(settingFile)) {
                    for (size_t i = 0; i < tagNames.size(); ++i) {
                        if (const auto *xmlColour = xmlInput->getChildByName(tagNames[i])) {
                            const juce::Colour colour = getIntColour(
                                xmlColour->getIntAttribute("r"),
                                xmlColour->getIntAttribute("g"),
                                xmlColour->getIntAttribute("b"),
                                static_cast<float>(xmlColour->getDoubleAttribute("o")));
                            uiBase.setColourByIdx(colourIdx[i], colour);
                        }
                    }
                    uiBase.saveToAPVTS();
                    loadSetting();
                }
            });
        } else if (event.originalComponent == &exportLabel) {
            myChooser = std::make_unique<juce::FileChooser>(
                "Save the colour settings...", settingDirectory.getChildFile("colour.xml"), "*.xml",
                true, false, nullptr);
            constexpr auto settingSaveFlags = juce::FileBrowserComponent::saveMode |
                                              juce::FileBrowserComponent::warnAboutOverwriting;
            myChooser->launchAsync(settingSaveFlags, [this](const juce::FileChooser &chooser) {
                if (chooser.getResults().size() <= 0) { return; }
                juce::File settingFile(chooser.getResult().withFileExtension("xml"));
                if (settingFile.create()) {
                    juce::XmlElement xmlOutput{"colour_setting"};
                    for (size_t i = 0; i < tagNames.size(); ++i) {
                        const auto tagName = selectorNames[i];
                        auto *xmlColour = xmlOutput.createNewChildElement(tagNames[i]);
                        juce::Colour colour = selectors[i]->getColour();
                        xmlColour->setAttribute("r", colour.getRed());
                        xmlColour->setAttribute("g", colour.getGreen());
                        xmlColour->setAttribute("b", colour.getBlue());
                        xmlColour->setAttribute("o", colour.getFloatAlpha());
                    }
                    const auto result = xmlOutput.writeTo(settingFile);
                    juce::ignoreUnused(result);
                }
            });
        }
    }
} // zlPanel
