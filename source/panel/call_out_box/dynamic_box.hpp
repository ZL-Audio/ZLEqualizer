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
              ui_base_(base),
              lookaheadS("Lookahead", ui_base_, zlgui::multilingual::Labels::kLookahead),
              rmsS("RMS", ui_base_, zlgui::multilingual::Labels::kRMS),
              smoothS("Smooth", ui_base_, zlgui::multilingual::Labels::kSmooth),
              dynHQC("HQ:", zlp::dynHQ::choices, ui_base_, zlgui::multilingual::Labels::kHighQuality) {
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
                c->setLabelPos(zlgui::ClickCombobox::kLeft);
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
            ui_base_.getBoxTree().addListener(this);
        }

        ~DynamicBox() override {
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
            const auto boxHeight = juce::roundToInt(boxHeightP * ui_base_.getFontSize());
            return {buttonWidth * 3 + padSize * 2, buttonHeight * 3 + boxHeight + padSize};
        }

        void resized() override {
            for (auto &c: {&lookaheadS, &rmsS, &smoothS}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto buttonHeight = static_cast<int>(buttonHeightP * ui_base_.getFontSize());

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
        zlgui::UIBase &ui_base_;

        zlgui::CompactLinearSlider lookaheadS, rmsS, smoothS;
        zlgui::ClickCombobox dynHQC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kDynamicBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kDynamicBox));
                setVisible(f);
            }
        }
    };
}
