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
              lookahead_s_("Lookahead", ui_base_, zlgui::multilingual::Labels::kLookahead),
              rms_s_("RMS", ui_base_, zlgui::multilingual::Labels::kRMS),
              smooth_s_("Smooth", ui_base_, zlgui::multilingual::Labels::kSmooth),
              dyn_hq_c_("HQ:", zlp::dynHQ::choices, ui_base_, zlgui::multilingual::Labels::kHighQuality) {
            for (auto &c: {&lookahead_s_, &rms_s_, &smooth_s_}) {
                addAndMakeVisible(c);
            }
            attach({
                       &lookahead_s_.getSlider(), &rms_s_.getSlider(), &smooth_s_.getSlider()
                   },
                   {
                       zlp::dynLookahead::ID, zlp::dynRMS::ID, zlp::dynSmooth::ID
                   },
                   parameters_ref_, slider_attachments_);
            for (auto &c: {&dyn_hq_c_}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlgui::ClickCombobox::kLeft);
                addAndMakeVisible(c);
            }
            attach({
                       &dyn_hq_c_.getCompactBox().getBox(),
                   },
                   {
                       zlp::dynHQ::ID
                   },
                   parameters_ref_, box_attachments_);
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
            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto button_height = static_cast<int>(kButtonHeightP * ui_base_.getFontSize());
            const auto button_width = static_cast<int>(ui_base_.getFontSize() * 2.5);
            const auto box_height = juce::roundToInt(kBoxHeightP * ui_base_.getFontSize());
            return {button_width * 3 + pad_size * 2, button_height * 3 + box_height + pad_size};
        }

        void resized() override {
            for (auto &c: {&lookahead_s_, &rms_s_, &smooth_s_}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto button_height = static_cast<int>(kButtonHeightP * ui_base_.getFontSize());

            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + pad_size, bound.getY(),
                                         bound.getWidth() - pad_size * 2, bound.getHeight() - pad_size);

            lookahead_s_.setBounds(bound.removeFromTop(button_height));
            rms_s_.setBounds(bound.removeFromTop(button_height));
            smooth_s_.setBounds(bound.removeFromTop(button_height));
            dyn_hq_c_.setBounds(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::CompactLinearSlider lookahead_s_, rms_s_, smooth_s_;
        zlgui::ClickCombobox dyn_hq_c_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> slider_attachments_{};
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> box_attachments_{};

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(tree_whose_property_has_changed);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kDynamicBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kDynamicBox));
                setVisible(f);
            }
        }
    };
}
