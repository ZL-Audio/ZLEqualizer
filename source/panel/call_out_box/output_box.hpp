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
    class OutputBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit OutputBox(PluginProcessor &p,
                                  zlgui::UIBase &base)
            : processor_ref_(p), parameters_ref_(p.parameters),
              ui_base_(base),
              phaseC("phase", ui_base_, zlgui::multilingual::Labels::kPhaseFlip),
              agcC("A", ui_base_, zlgui::multilingual::Labels::kAutoGC),
              lmC("L", ui_base_, zlgui::multilingual::Labels::kLoudnessMatch),
              scaleS("Scale", ui_base_, zlgui::multilingual::Labels::kScale),
              outGainS("Out Gain", ui_base_, zlgui::multilingual::Labels::kOutputGain),
              phaseDrawable(
                  juce::Drawable::createFromImageData(BinaryData::fadphase_svg,
                                                      BinaryData::fadphase_svgSize)),
              agcDrawable(juce::Drawable::createFromImageData(BinaryData::autogaincompensation_svg,
                                                              BinaryData::autogaincompensation_svgSize)),
              lmDrawable(juce::Drawable::createFromImageData(BinaryData::loudnessmatch_svg,
                                                             BinaryData::loudnessmatch_svgSize)),
              agcUpdater(p.parameters, zlp::autoGain::ID),
              gainUpdater(p.parameters, zlp::outputGain::ID) {
            setBufferedToImage(true);
            phaseC.setDrawable(phaseDrawable.get());
            agcC.setDrawable(agcDrawable.get());
            lmC.setDrawable(lmDrawable.get());

            for (auto &c: {&phaseC, &agcC, &lmC}) {
                c->getLAF().enableShadow(false);
                c->getLAF().setShrinkScale(0.f);
                addAndMakeVisible(c);
            }
            for (auto &c: {&scaleS, &outGainS}) {
                addAndMakeVisible(c);
            }
            attach({&phaseC.getButton(), &agcC.getButton(), &lmC.getButton()},
                   {zlp::phaseFlip::ID, zlp::autoGain::ID, zlp::loudnessMatcherON::ID},
                   parameters_ref_, buttonAttachments);
            attach({&scaleS.getSlider(), &outGainS.getSlider()},
                   {zlp::scale::ID, zlp::outputGain::ID},
                   parameters_ref_, sliderAttachments);

            lmC.getButton().onClick = [this]() {
                if (!lmC.getButton().getToggleState()) {
                    const auto newGain = -processor_ref_.getController().getLoudnessMatcherDiff();
                    agcUpdater.updateSync(0.f);
                    gainUpdater.updateSync(zlp::outputGain::convertTo01(static_cast<float>(newGain)));
                }
            };

            ui_base_.getBoxTree().addListener(this);
        }

        ~OutputBox() override {
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
            const auto buttonHeight = static_cast<int>(buttonHeightP * ui_base_.getFontSize());
            const auto buttonWidth = static_cast<int>(ui_base_.getFontSize() * 2.5);
            return {buttonWidth * 3 + padSize * 2, buttonHeight * 3 + padSize};
        }

        void resized() override {
            for (auto &c: {&scaleS, &outGainS}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.5f);
            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50)), Track(Fr(50)), Track(Fr(50))};

            grid.items = {
                juce::GridItem(scaleS).withArea(1, 1, 2, 4),
                juce::GridItem(phaseC).withArea(2, 1),
                juce::GridItem(agcC).withArea(2, 2),
                juce::GridItem(lmC).withArea(2, 3),
                juce::GridItem(outGainS).withArea(3, 1, 4, 4)
            };

            grid.performLayout(bound);
        }

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::CompactButton phaseC, agcC, lmC;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > buttonAttachments{};

        zlgui::CompactLinearSlider scaleS, outGainS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        const std::unique_ptr<juce::Drawable> phaseDrawable;
        const std::unique_ptr<juce::Drawable> agcDrawable;
        const std::unique_ptr<juce::Drawable> lmDrawable;

        zldsp::chore::ParaUpdater agcUpdater, gainUpdater;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged, property);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kOutputBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kOutputBox));
                setVisible(f);
            }
        }
    };
}
