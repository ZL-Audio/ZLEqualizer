// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_control_panel.hpp"

namespace zlPanel {
    MatchControlPanel::MatchControlPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : uiBase(base), analyzer(p.getController().getMatchAnalyzer()),
          startDrawable(juce::Drawable::createFromImageData(BinaryData::playfill_svg, BinaryData::playfill_svgSize)),
          pauseDrawable(juce::Drawable::createFromImageData(BinaryData::pauseline_svg, BinaryData::pauseline_svgSize)),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          sideChooseBox("", {"Side", "Preset", "Flat"}, base),
          fitAlgoBox("", {"LD", "GN"}, base),
          weightSlider("Weight", base),
          smoothSlider("Smooth", base),
          slopeSlider("Slope", base),
          numBandSlider("Num Band", base),
          learnButton(base, startDrawable.get(), pauseDrawable.get()),
          saveButton(base, saveDrawable.get()),
          fitButton(base, startDrawable.get()) {
        juce::ignoreUnused(p);
        uiBase.getValueTree().addListener(this);

        sideChooseBox.getBox().setSelectedId(1);
        fitAlgoBox.getBox().setSelectedId(1);
        for (const auto &c: {&sideChooseBox, &fitAlgoBox}) {
            addAndMakeVisible(c);
        }
        weightSlider.getSlider().setRange(0.0, 1.0, 0.01);
        weightSlider.getSlider().setValue(0.5);
        smoothSlider.getSlider().setRange(0.0, 1.0, 0.01);
        smoothSlider.getSlider().setValue(0.5);
        slopeSlider.getSlider().setRange(-4.5, 4.5, 0.01);
        slopeSlider.getSlider().setValue(0.0);
        numBandSlider.getSlider().setRange(1.0, 16.0, 1.0);
        numBandSlider.getSlider().setValue(8.0);
        for (const auto &c: {&weightSlider, &smoothSlider, &slopeSlider, &numBandSlider}) {
            addAndMakeVisible(c);
        }
        for (const auto &c: {&learnButton, &saveButton, &fitButton}) {
            addAndMakeVisible(c);
            c->setPadding(.2f, .2f, .2f, .2f);
        }

        learnButton.getButton().onClick = [this]() {
            analyzer.setON(learnButton.getButton().getToggleState());
        };
    }

    MatchControlPanel::~MatchControlPanel() {
        uiBase.getValueTree().removeListener(this);
        analyzer.setON(false);
    }

    void MatchControlPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        g.fillAll(uiBase.getColourByIdx(zlInterface::colourIdx::backgroundColour));
        bound = bound.removeFromLeft(bound.getWidth() * weightP);
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void MatchControlPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(60)), Track(Fr(30)), Track(Fr(60)),
            Track(Fr(30)), Track(Fr(30))
        };

        grid.items = {
            juce::GridItem(sideChooseBox).withArea(1, 1),
            juce::GridItem(weightSlider).withArea(2, 1),
            juce::GridItem(learnButton).withArea(1, 2),
            juce::GridItem(saveButton).withArea(2, 2),
            juce::GridItem(smoothSlider).withArea(1, 3),
            juce::GridItem(slopeSlider).withArea(2, 3),
            juce::GridItem(fitAlgoBox).withArea(1, 4),
            juce::GridItem(fitButton).withArea(1, 5),
            juce::GridItem(numBandSlider).withArea(2, 4, 3, 6)
        };

        for (const auto &c: {&weightSlider, &smoothSlider, &slopeSlider, &numBandSlider}) {
            c->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }

        auto bound = getLocalBounds().toFloat();
        bound = bound.removeFromLeft(bound.getWidth() * weightP);
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
    }

    void MatchControlPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) {
        setVisible(static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow)));
    }
} // zlPanel
