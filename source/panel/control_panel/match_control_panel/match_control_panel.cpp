// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_control_panel.hpp"

namespace zlpanel {
    MatchControlPanel::MatchControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : uiBase(base), analyzer(p.getController().getMatchAnalyzer()),
          startDrawable(juce::Drawable::createFromImageData(BinaryData::playfill_svg, BinaryData::playfill_svgSize)),
          pauseDrawable(juce::Drawable::createFromImageData(BinaryData::pauseline_svg, BinaryData::pauseline_svgSize)),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          sideChooseBox("", {"Side", "Preset", "Flat"}, base, zlgui::multilingual::labels::matchTarget),
          fitAlgoBox("", {"LD", "GN", "GN+"}, base, zlgui::multilingual::labels::matchAlgo),
          weightSlider("Weight", base, zlgui::multilingual::labels::matchWeight),
          smoothSlider("Smooth", base, zlgui::multilingual::labels::matchSmooth),
          slopeSlider("Slope", base, zlgui::multilingual::labels::matchSlope),
          numBandSlider("Num Band", base, zlgui::multilingual::labels::matchNumBand),
          learnButton(base, startDrawable.get(), pauseDrawable.get(),
                      zlgui::multilingual::labels::matchStartLearn),
          saveButton(base, saveDrawable.get(), nullptr, zlgui::multilingual::labels::matchSave),
          fitButton(base, startDrawable.get(), nullptr, zlgui::multilingual::labels::matchStartFit),
          matchRunner(p, uiBase, analyzer.getDiffs(), numBandSlider) {
        uiBase.getValueTree().addListener(this);
        // create preset directory if not exists
        if (!presetDirectory.isDirectory()) {
            const auto f = presetDirectory.createDirectory();
            juce::ignoreUnused(f);
        }
        // init combobox
        const auto *menu = sideChooseBox.getBox().getRootMenu();
        juce::PopupMenu::MenuItemIterator iterator(*menu);
        while (iterator.next()) {
            auto item = &iterator.getItem();
            if (item->itemID == 1) {
                item->setAction([this] {
                    analyzer.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::matchSide);
                });
            } else if (item->itemID == 2) {
                item->setAction([this] {
                    loadFromPreset();
                    analyzer.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::matchPreset);
                });
            } else if (item->itemID == 3) {
                item->setAction([this] {
                    analyzer.setTargetSlope(0.f);
                    analyzer.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::matchSlope);
                });
            }
        }
        fitAlgoBox.getBox().onChange = [this]() {
            const auto fitAlgo = static_cast<size_t>(fitAlgoBox.getBox().getSelectedId() - 1);
            matchRunner.setMode(fitAlgo);
        };
        for (const auto &c: {&sideChooseBox, &fitAlgoBox}) {
            addAndMakeVisible(c);
        }
        weightSlider.getSlider().setRange(0.0, 1.0, 0.01);
        weightSlider.getSlider().setDoubleClickReturnValue(true, .5);
        weightSlider.getSlider().onValueChange = [this]() {
            analyzer.getAverageFFT().setWeight(static_cast<float>(weightSlider.getSlider().getValue()));
        };
        smoothSlider.getSlider().setRange(0.0, 1.0, 0.01);
        smoothSlider.getSlider().setDoubleClickReturnValue(true, .5);
        smoothSlider.getSlider().onValueChange = [this]() {
            analyzer.setSmooth(static_cast<float>(smoothSlider.getSlider().getValue()));
        };
        slopeSlider.getSlider().setRange(-4.5, 4.5, 0.01);
        slopeSlider.getSlider().setDoubleClickReturnValue(true, 0.);
        slopeSlider.getSlider().onValueChange = [this]() {
            analyzer.setSlope(static_cast<float>(slopeSlider.getSlider().getValue()));
        };
        numBandSlider.getSlider().onValueChange = [this]() {
            matchRunner.setNumBand(static_cast<size_t>(numBandSlider.getSlider().getValue()));
            matchRunner.update();
        };
        for (const auto &c: {
                 &weightSlider, &smoothSlider, &slopeSlider, &numBandSlider
             }) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
        }
        for (const auto &c: {&learnButton, &saveButton, &fitButton}) {
            c->setBufferedToImage(true);
            addAndMakeVisible(c);
            c->setPadding(.2f, .2f, .2f, .2f);
        }
        learnButton.getButton().onClick = [this]() {
            analyzer.setON(learnButton.getButton().getToggleState());
            uiBase.setProperty(zlgui::settingIdx::matchPanelFit, false);
        };
        saveButton.getButton().onClick = [this]() {
            learnButton.getButton().setToggleState(false, juce::dontSendNotification);
            analyzer.setON(false);
            saveToPreset();
        };
        fitButton.getButton().onClick = [this]() {
            learnButton.getButton().setToggleState(false, juce::dontSendNotification);
            analyzer.setON(false);
            matchRunner.start();
            uiBase.setProperty(zlgui::settingIdx::matchPanelFit, true);
            uiBase.setProperty(zlgui::settingIdx::matchFitRunning, true);
        };
        resetDefault();

        setOpaque(true);
    }

    MatchControlPanel::~MatchControlPanel() {
        uiBase.getValueTree().removeListener(this);
        analyzer.setON(false);
    }

    void MatchControlPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getColourByIdx(zlgui::colourIdx::backgroundColour));
        uiBase.fillRoundedShadowRectangle(g,
                                          internalBound.toFloat(),
                                          0.5f * uiBase.getFontSize(),
                                          {.blurRadius = 0.25f});
    }

    void MatchControlPanel::resized() {
        // update padding
        {
            for (const auto &c: {
                     &weightSlider, &smoothSlider, &slopeSlider, &numBandSlider
                 }) {
                c->setPadding(std::round(uiBase.getFontSize() * 0.5f),
                              std::round(uiBase.getFontSize() * 0.6f));
            }
        }
        // update bounds
        auto bound = getLocalBounds();
        const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * buttonWidthP);
        const auto sliderWidth = static_cast<int>(std::round(uiBase.getFontSize() * rSliderWidthP * 1.1f)); {
            const auto pad = static_cast<int>(uiBase.getFontSize() * .5f);
            internalBound = bound.withSizeKeepingCentre(buttonWidth + sliderWidth * 3 + 2 * pad, bound.getHeight());
            bound = internalBound;
            bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * pad, bound.getHeight() - 2 * pad);
        }
        const auto buttonHeight = std::min(static_cast<int>(uiBase.getFontSize() * buttonHeightP),
                                           bound.getHeight() / 2); {
            auto mBound = bound.removeFromLeft(sliderWidth);
            sideChooseBox.setBounds(mBound.removeFromTop(buttonHeight));
            weightSlider.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(buttonWidth);
            learnButton.setBounds(mBound.removeFromTop(buttonHeight));
            saveButton.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(sliderWidth);
            smoothSlider.setBounds(mBound.removeFromTop(buttonHeight));
            slopeSlider.setBounds(mBound.removeFromBottom(buttonHeight));
        } {
            auto mBound = bound.removeFromLeft(sliderWidth);
            numBandSlider.setBounds(mBound.removeFromBottom(buttonHeight));
            mBound = mBound.removeFromTop(buttonHeight);
            fitAlgoBox.setBounds(mBound.removeFromLeft(buttonWidth));
            fitButton.setBounds(mBound.removeFromRight(buttonWidth));
        }
    }

    void MatchControlPanel::resetDefault() {
        weightSlider.getSlider().setValue(0.5, juce::dontSendNotification);
        weightSlider.updateDisplayValue();
        analyzer.getAverageFFT().setWeight(.5f);

        smoothSlider.getSlider().setValue(0.5, juce::dontSendNotification);
        smoothSlider.updateDisplayValue();
        analyzer.setSmooth(.5f);

        slopeSlider.getSlider().setValue(0., juce::dontSendNotification);
        slopeSlider.updateDisplayValue();
        analyzer.setSlope(0.f);

        learnButton.getButton().setToggleState(false, juce::dontSendNotification);
        analyzer.setON(false);
        analyzer.reset();

        numBandSlider.getSlider().setRange(1.0, 16.0, 1.0);
        numBandSlider.getSlider().setValue(8.0, juce::dontSendNotification);
        numBandSlider.updateDisplayValue();
        matchRunner.setNumBand(static_cast<size_t>(8));

        sideChooseBox.getBox().setSelectedId(1, juce::dontSendNotification);
        analyzer.setMatchMode(zldsp::eq_match::EqMatchAnalyzer<double>::MatchMode::matchSide);

        fitAlgoBox.getBox().setSelectedId(2, juce::dontSendNotification);
        matchRunner.setMode(static_cast<size_t>(1));
    }

    void MatchControlPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) {
        const auto f = static_cast<bool>(uiBase.getProperty(zlgui::settingIdx::matchPanelShow));
        setVisible(f);
        if (!f) {
            resetDefault();
        }
    }

    void MatchControlPanel::loadFromPreset() {
        myChooser = std::make_unique<juce::FileChooser>(
            "Load the match preset...", presetDirectory, "*.csv",
            true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (!settingFile.existsAsFile()) { return; }
            const auto stream(settingFile.createInputStream());
            if (stream->isExhausted()) { return; }
            const auto start = stream->readNextLine();
            if (start.startsWith("#native")) {
                std::array<float, 251> points{};
                size_t idx = 0;
                while (!stream->isExhausted() && idx < points.size()) {
                    points[idx] = stream->readNextLine().getFloatValue();
                    idx += 1;
                }
                if (idx == points.size()) {
                    analyzer.setTargetPreset(points);
                }
            }
        });
    }

    void MatchControlPanel::saveToPreset() {
        myChooser = std::make_unique<juce::FileChooser>(
            "Save the match preset...", presetDirectory.getChildFile("match.csv"), "*.csv",
            true, false, nullptr);
        constexpr auto settingSaveFlags = juce::FileBrowserComponent::saveMode |
                                          juce::FileBrowserComponent::warnAboutOverwriting;
        myChooser->launchAsync(settingSaveFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            juce::File settingFile(chooser.getResult().withFileExtension("csv"));
            if (settingFile.existsAsFile()) {
                const auto f = settingFile.deleteFile();
                juce::ignoreUnused(f);
            }
            const auto stream = settingFile.createOutputStream();
            stream->writeText("#native", false, false, nullptr);
            stream->writeText(",\n", false, false, nullptr);
            for (auto &p: analyzer.getTarget()) {
                stream->writeText(juce::String(p.load()), false, false, nullptr);
                stream->writeText(",\n", false, false, nullptr);
            }
        });
    }
} // zlpanel
