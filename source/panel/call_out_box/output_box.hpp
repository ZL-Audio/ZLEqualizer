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

namespace zlPanel {
    class OutputBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit OutputBox(PluginProcessor &p,
                                  zlInterface::UIBase &base)
            : processorRef(p), parametersRef(p.parameters),
              uiBase(base),
              phaseC("phase", uiBase, zlInterface::multilingual::labels::phaseFlip),
              agcC("A", uiBase, zlInterface::multilingual::labels::autoGC),
              lmC("L", uiBase, zlInterface::multilingual::labels::loudnessMatch),
              scaleS("Scale", uiBase, zlInterface::multilingual::labels::scale),
              outGainS("Out Gain", uiBase, zlInterface::multilingual::labels::outputGain),
              phaseDrawable(
                  juce::Drawable::createFromImageData(BinaryData::fadphase_svg,
                                                      BinaryData::fadphase_svgSize)),
              agcDrawable(juce::Drawable::createFromImageData(BinaryData::autogaincompensation_svg,
                                                              BinaryData::autogaincompensation_svgSize)),
              lmDrawable(juce::Drawable::createFromImageData(BinaryData::loudnessmatch_svg,
                                                             BinaryData::loudnessmatch_svgSize)),
              agcUpdater(p.parameters, zlDSP::autoGain::ID),
              gainUpdater(p.parameters, zlDSP::outputGain::ID) {
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
                   {zlDSP::phaseFlip::ID, zlDSP::autoGain::ID, zlDSP::loudnessMatcherON::ID},
                   parametersRef, buttonAttachments);
            attach({&scaleS.getSlider(), &outGainS.getSlider()},
                   {zlDSP::scale::ID, zlDSP::outputGain::ID},
                   parametersRef, sliderAttachments);

            lmC.getButton().onClick = [this]() {
                if (!lmC.getButton().getToggleState()) {
                    const auto newGain = -processorRef.getController().getLoudnessMatcherDiff();
                    agcUpdater.updateSync(0.f);
                    gainUpdater.updateSync(zlDSP::outputGain::convertTo01(static_cast<float>(newGain)));
                }
            };

            uiBase.getBoxTree().addListener(this);
        }

        ~OutputBox() override {
            uiBase.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            g.fillAll(uiBase.getBackgroundColor());
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto buttonHeight = static_cast<int>(buttonHeightP * uiBase.getFontSize());
            const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * 2.5);
            return {buttonWidth * 3, buttonHeight * 3 + padSize};
        }

        void resized() override {
            for (auto &c: {&scaleS, &outGainS}) {
                c->setPadding(std::round(uiBase.getFontSize() * 0.5f),
                              std::round(uiBase.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.5f);
            auto bound = getLocalBounds();
            bound = bound.withSize(bound.getWidth(), bound.getHeight() - padSize);

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
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactButton phaseC, agcC, lmC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<false> > buttonAttachments{};

        zlInterface::CompactLinearSlider scaleS, outGainS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        const std::unique_ptr<juce::Drawable> phaseDrawable;
        const std::unique_ptr<juce::Drawable> agcDrawable;
        const std::unique_ptr<juce::Drawable> lmDrawable;

        zlChore::ParaUpdater agcUpdater, gainUpdater;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged, property);
            if (uiBase.isBoxProperty(zlInterface::boxIdx::outputBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlInterface::boxIdx::outputBox));
                setVisible(f);
            }
        }
    };
}
