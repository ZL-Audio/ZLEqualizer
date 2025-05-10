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
    class DynamicBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit DynamicBox(juce::AudioProcessorValueTreeState &parameters, zlgui::UIBase &base)
            : parameters_ref_(parameters),
              uiBase(base),
              lookaheadS("Lookahead", uiBase, zlgui::multilingual::labels::lookahead),
              rmsS("RMS", uiBase, zlgui::multilingual::labels::rms),
              smoothS("Smooth", uiBase, zlgui::multilingual::labels::smooth),
              dynHQC("HQ:", zlp::dynHQ::choices, uiBase, zlgui::multilingual::labels::highQuality) {
            for (auto &c: {&lookaheadS, &rmsS, &smoothS}) {
                addAndMakeVisible(c);
            }
            attach({
                       &lookaheadS.getSlider(), &rmsS.getSlider(), &smoothS.getSlider()
                   },
                   {
                       zlp::dynLookahead::ID, zlp::dynRMS::ID, zlp::dynSmooth::ID
                   },
                   parameters_ref_, sliderAttachments);
            for (auto &c: {&dynHQC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlgui::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &dynHQC.getCompactBox().getBox(),
                   },
                   {
                       zlp::dynHQ::ID
                   },
                   parameters_ref_, boxAttachments);
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
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &uiBase;

        zlgui::CompactLinearSlider lookaheadS, rmsS, smoothS;
        zlgui::ClickCombobox dynHQC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlgui::boxIdx::dynamicBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlgui::boxIdx::dynamicBox));
                setVisible(f);
            }
        }
    };
}
