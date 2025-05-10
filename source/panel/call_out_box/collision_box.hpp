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
    class CollisionBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit CollisionBox(juce::AudioProcessorValueTreeState &parameters_NA,
                              zlgui::UIBase &base)
            : parameters_NA_ref_(parameters_NA),
              ui_base_(base),
              collisionC("DET:", zlstate::conflictON::choices, ui_base_, zlgui::multilingual::Labels::kCollisionDET),
              strengthS("Strength", ui_base_, zlgui::multilingual::Labels::kCollisionStrength),
              scaleS("Scale", ui_base_, zlgui::multilingual::Labels::kCollisionScale) {
            collisionC.getLabelLAF().setFontScale(1.5f);
            collisionC.setLabelScale(.5f);
            collisionC.setLabelPos(zlgui::ClickCombobox::kLeft);
            addAndMakeVisible(collisionC);
            for (auto &c: {&strengthS, &scaleS}) {
                c->setPadding(ui_base_.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&collisionC.getCompactBox().getBox()},
                   {zlstate::conflictON::ID},
                   parameters_NA_ref_, boxAttachments);
            attach({&strengthS.getSlider(), &scaleS.getSlider()},
                   {
                       zlstate::conflictStrength::ID,
                       zlstate::conflictScale::ID
                   },
                   parameters_NA_ref_, sliderAttachments);
            setBufferedToImage(true);

            ui_base_.getBoxTree().addListener(this);
        }

        ~CollisionBox() override {
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
            return {buttonWidth * 3 + padSize * 2, buttonHeight * 2 + boxHeight + padSize};
        }

        void resized() override {
            for (auto &c: {&strengthS, &scaleS}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto buttonHeight = static_cast<int>(buttonHeightP * ui_base_.getFontSize());

            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            scaleS.setBounds(bound.removeFromBottom(buttonHeight));
            strengthS.setBounds(bound.removeFromBottom(buttonHeight));
            collisionC.setBounds(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::ClickCombobox collisionC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        zlgui::CompactLinearSlider strengthS, scaleS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kCollisionBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kCollisionBox));
                setVisible(f);
            }
        }
    };
}
