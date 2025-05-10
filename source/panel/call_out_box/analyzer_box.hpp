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
        explicit AnalyzerBox(juce::AudioProcessorValueTreeState &parameters_NA,
                               zlgui::UIBase &base)
            : parameters_NA_ref_(parameters_NA),
              ui_base_(base),
              fftPreON("Pre:", zlstate::fftPreON::choices, ui_base_, zlgui::multilingual::Labels::kFFTPre),
              fftPostON("Post:", zlstate::fftPostON::choices, ui_base_, zlgui::multilingual::Labels::kFFTPost),
              fftSideON("Side:", zlstate::fftSideON::choices, ui_base_, zlgui::multilingual::Labels::kFFTSide),
              ffTSpeed("", zlstate::ffTSpeed::choices, ui_base_, zlgui::multilingual::Labels::kFFTDecay),
              fftTilt("", zlstate::ffTTilt::choices, ui_base_, zlgui::multilingual::Labels::kFFTSlope) {
            for (auto &c: {&fftPreON, &fftPostON, &fftSideON}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlgui::ClickCombobox::kLeft);
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
                   parameters_NA_ref_, boxAttachments);
            setBufferedToImage(true);

            ui_base_.getBoxTree().addListener(this);
        }

        ~AnalyzerBox() override {
            ui_base_.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            juce::Path path;
            const auto bound = getLocalBounds().toFloat();
            path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                     std::round(ui_base_.getFontSize() * .25f),
                                     std::round(ui_base_.getFontSize() * .25f),
                                     false, false, true, true);
            g.setColour(ui_base_.getBackgroundColor());
            g.fillPath(path);
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto buttonWidth = static_cast<int>(ui_base_.getFontSize() * 2.5);
            const auto boxHeight = juce::roundToInt(boxHeightP * ui_base_.getFontSize());
            return {buttonWidth * 3 + padSize * 2, boxHeight * 5 + padSize};
        }

        void resized() override {
            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
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
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::ClickCombobox fftPreON, fftPostON, fftSideON;
        zlgui::CompactCombobox ffTSpeed, fftTilt;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kAnalyzerBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kAnalyzerBox));
                setVisible(f);
            }
        }
    };
}
