// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class AnalyzerBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit AnalyzerBox(juce::AudioProcessorValueTreeState &parametersNA,
                               zlgui::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base),
              fftPreON("Pre:", zlstate::fftPreON::choices, uiBase, zlgui::multilingual::labels::fftPre),
              fftPostON("Post:", zlstate::fftPostON::choices, uiBase, zlgui::multilingual::labels::fftPost),
              fftSideON("Side:", zlstate::fftSideON::choices, uiBase, zlgui::multilingual::labels::fftSide),
              ffTSpeed("", zlstate::ffTSpeed::choices, uiBase, zlgui::multilingual::labels::fftDecay),
              fftTilt("", zlstate::ffTTilt::choices, uiBase, zlgui::multilingual::labels::fftSlope) {
            for (auto &c: {&fftPreON, &fftPostON, &fftSideON}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlgui::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            for (auto &c: {&ffTSpeed, &fftTilt}) {
                addAndMakeVisible(c);
            }
            attach({
                       &fftPreON.getCompactBox().getBox(),
                       &fftPostON.getCompactBox().getBox(),
                       &fftSideON.getCompactBox().getBox(),
                       &ffTSpeed.getBox(), &fftTilt.getBox()
                   },
                   {
                       zlstate::fftPreON::ID, zlstate::fftPostON::ID, zlstate::fftSideON::ID,
                       zlstate::ffTSpeed::ID, zlstate::ffTTilt::ID
                   },
                   parametersNARef, boxAttachments);
            setBufferedToImage(true);

            uiBase.getBoxTree().addListener(this);
        }

        ~AnalyzerBox() override {
            uiBase.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            juce::Path path;
            const auto bound = getLocalBounds().toFloat();
            path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                     std::round(uiBase.getFontSize() * .25f),
                                     std::round(uiBase.getFontSize() * .25f),
                                     false, false, true, true);
            g.setColour(uiBase.getBackgroundColor());
            g.fillPath(path);
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * 2.5);
            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            return {buttonWidth * 3 + padSize * 2, boxHeight * 5 + padSize};
        }

        void resized() override {
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(fftPreON).withArea(1, 1),
                juce::GridItem(fftPostON).withArea(2, 1),
                juce::GridItem(fftSideON).withArea(3, 1),
                juce::GridItem(ffTSpeed).withArea(4, 1),
                juce::GridItem(fftTilt).withArea(5, 1)
            };
            grid.performLayout(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlgui::UIBase &uiBase;

        zlgui::ClickCombobox fftPreON, fftPostON, fftSideON;
        zlgui::CompactCombobox ffTSpeed, fftTilt;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlgui::boxIdx::analyzerBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlgui::boxIdx::analyzerBox));
                setVisible(f);
            }
        }
    };
}
