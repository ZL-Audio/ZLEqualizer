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
    class DynamicBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit DynamicBox(juce::AudioProcessorValueTreeState &parameters, zlInterface::UIBase &base)
            : parametersRef(parameters),
              uiBase(base),
              lookaheadS("Lookahead", uiBase, zlInterface::multilingual::labels::lookahead),
              rmsS("RMS", uiBase, zlInterface::multilingual::labels::rms),
              smoothS("Smooth", uiBase, zlInterface::multilingual::labels::smooth),
              dynHQC("HQ:", zlDSP::dynHQ::choices, uiBase, zlInterface::multilingual::labels::highQuality) {
            for (auto &c: {&lookaheadS, &rmsS, &smoothS}) {
                addAndMakeVisible(c);
            }
            attach({
                       &lookaheadS.getSlider(), &rmsS.getSlider(), &smoothS.getSlider()
                   },
                   {
                       zlDSP::dynLookahead::ID, zlDSP::dynRMS::ID, zlDSP::dynSmooth::ID
                   },
                   parametersRef, sliderAttachments);
            for (auto &c: {&dynHQC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlInterface::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &dynHQC.getCompactBox().getBox(),
                   },
                   {
                       zlDSP::dynHQ::ID
                   },
                   parametersRef, boxAttachments);
            setBufferedToImage(true);
            uiBase.getBoxTree().addListener(this);
        }

        ~DynamicBox() override {
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
            const auto buttonHeight = static_cast<int>(buttonHeightP * uiBase.getFontSize());
            const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * 2.5);
            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            return {buttonWidth * 3 + padSize * 2, buttonHeight * 3 + boxHeight + padSize};
        }

        void resized() override {
            for (auto &c: {&lookaheadS, &rmsS, &smoothS}) {
                c->setPadding(std::round(uiBase.getFontSize() * 0.5f),
                              std::round(uiBase.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto buttonHeight = static_cast<int>(buttonHeightP * uiBase.getFontSize());

            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            lookaheadS.setBounds(bound.removeFromTop(buttonHeight));
            rmsS.setBounds(bound.removeFromTop(buttonHeight));
            smoothS.setBounds(bound.removeFromTop(buttonHeight));
            dynHQC.setBounds(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactLinearSlider lookaheadS, rmsS, smoothS;
        zlInterface::ClickCombobox dynHQC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlInterface::boxIdx::dynamicBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlInterface::boxIdx::dynamicBox));
                setVisible(f);
            }
        }
    };
}
