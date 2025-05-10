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
    class GeneralBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit GeneralBox(juce::AudioProcessorValueTreeState &parameters,
                            zlgui::UIBase &base)
            : parameters_ref_(parameters),
              ui_base_(base),
              filterStructure("", zlp::filterStructure::choices, ui_base_,
                              zlgui::multilingual::Labels::kFilterStructure,
                              {
                                  zlgui::multilingual::Labels::kMinimumPhase,
                                  zlgui::multilingual::Labels::kStateVariable,
                                  zlgui::multilingual::Labels::kParallelPhase,
                                  zlgui::multilingual::Labels::kMatchedPhase,
                                  zlgui::multilingual::Labels::kMixedPhase,
                                  zlgui::multilingual::Labels::kLinearPhase
                              }),
              zeroLATC("Zero LAT:", zlp::zeroLatency::choices, ui_base_,
                       zlgui::multilingual::Labels::kZeroLatency) {
            for (auto &c: {&filterStructure}) {
                addAndMakeVisible(c);
            }
            for (auto &c: {&zeroLATC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.625f);
                c->setLabelPos(zlgui::ClickCombobox::kLeft);
                addAndMakeVisible(c);
            }
            attach({
                       &filterStructure.getBox(), &zeroLATC.getCompactBox().getBox()
                   },
                   {
                       zlp::filterStructure::ID, zlp::zeroLatency::ID
                   },
                   parameters_ref_, boxAttachments);
            setBufferedToImage(true);

            ui_base_.getBoxTree().addListener(this);
        }

        ~GeneralBox() override {
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
            const auto boxHeight = juce::roundToInt(boxHeightP * ui_base_.getFontSize());
            return {static_cast<int>(ui_base_.getFontSize() * 10) + padSize * 2, boxHeight * 2 + padSize};
        }

        void resized() override {
            auto bound = getLocalBounds();
            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            const auto boxHeight = juce::roundToInt(boxHeightP * ui_base_.getFontSize());
            filterStructure.setBounds(bound.removeFromTop(boxHeight));
            zeroLATC.setBounds(bound.removeFromTop(boxHeight));
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::CompactCombobox filterStructure;
        zlgui::ClickCombobox zeroLATC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kGeneralBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kGeneralBox));
                setVisible(f);
            }
        }
    };
}
