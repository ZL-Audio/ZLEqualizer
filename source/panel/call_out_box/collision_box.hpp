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
              collision_c_("DET:", zlstate::conflictON::choices, ui_base_, zlgui::multilingual::Labels::kCollisionDET),
              strength_s_("Strength", ui_base_, zlgui::multilingual::Labels::kCollisionStrength),
              scale_s_("Scale", ui_base_, zlgui::multilingual::Labels::kCollisionScale) {
            collision_c_.getLabelLAF().setFontScale(1.5f);
            collision_c_.setLabelScale(.5f);
            collision_c_.setLabelPos(zlgui::ClickCombobox::kLeft);
            addAndMakeVisible(collision_c_);
            for (auto &c: {&strength_s_, &scale_s_}) {
                c->setPadding(ui_base_.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&collision_c_.getCompactBox().getBox()},
                   {zlstate::conflictON::ID},
                   parameters_NA_ref_, box_attachments_);
            attach({&strength_s_.getSlider(), &scale_s_.getSlider()},
                   {
                       zlstate::conflictStrength::ID,
                       zlstate::conflictScale::ID
                   },
                   parameters_NA_ref_, slider_attachments_);
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
            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto button_height = static_cast<int>(kButtonHeightP * ui_base_.getFontSize());
            const auto button_width = static_cast<int>(ui_base_.getFontSize() * 2.5);
            const auto box_height = juce::roundToInt(kBoxHeightP * ui_base_.getFontSize());
            return {button_width * 3 + pad_size * 2, button_height * 2 + box_height + pad_size};
        }

        void resized() override {
            for (auto &c: {&strength_s_, &scale_s_}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto button_height = static_cast<int>(kButtonHeightP * ui_base_.getFontSize());

            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + pad_size, bound.getY(),
                                         bound.getWidth() - pad_size * 2, bound.getHeight() - pad_size);

            scale_s_.setBounds(bound.removeFromBottom(button_height));
            strength_s_.setBounds(bound.removeFromBottom(button_height));
            collision_c_.setBounds(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::ClickCombobox collision_c_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> box_attachments_{};

        zlgui::CompactLinearSlider strength_s_, scale_s_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> slider_attachments_{};

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(tree_whose_property_has_changed);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kCollisionBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kCollisionBox));
                setVisible(f);
            }
        }
    };
}
