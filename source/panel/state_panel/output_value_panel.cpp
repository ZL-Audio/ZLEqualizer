// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_value_panel.hpp"
#include "../../dsp/dsp.hpp"

namespace zlpanel {
    OutputValuePanel::OutputValuePanel(PluginProcessor &p,
                                       zlgui::UIBase &base)
        : processor_ref_(p),
          parameters_ref_(p.parameters_),
          parameters_NA_ref_(p.parameters_NA_),
          ui_base_(base),
          scale_(*parameters_ref_.getRawParameterValue(zlp::scale::ID)) {
        juce::ignoreUnused(parameters_ref_, parameters_NA_ref_);
        lm_para_ = parameters_ref_.getParameter(zlp::loudnessMatcherON::ID);
        lookAndFeelChanged();
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);

        ui_base_.getBoxTree().addListener(this);
    }

    OutputValuePanel::~OutputValuePanel() {
        ui_base_.getBoxTree().removeListener(this);
        stopTimer(0);
    }

    void OutputValuePanel::paint(juce::Graphics &g) {
        g.setFont(ui_base_.getFontSize() * 1.375f);
        if (show_gain_) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kGainColour));
            g.drawText(gain_string_, gain_bound_, juce::Justification::centred);
            g.drawText(scale_string_, scale_bound_, juce::Justification::centred);
        } else {
            if (static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kOutputBox))) {
                g.setColour(ui_base_.getTextColor());
            } else {
                g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.75f));
            }
            g.drawText("Output", getLocalBounds().toFloat(), juce::Justification::centred);
        }
    }

    void OutputValuePanel::timerCallback(const int timerID) {
        if (!ui_base_.getIsEditorShowing()) return;
        if (timerID == 0) {
            updateGainValue();
        }
    }

    void OutputValuePanel::lookAndFeelChanged() {
        if (ui_base_.getColourByIdx(zlgui::kGainColour).getAlpha() > juce::uint8(0)) {
            show_gain_ = true;
            startTimer(0, 1500);
        } else {
            stopTimer(0);
            show_gain_ = false;
            repaint();
        }
    }

    void OutputValuePanel::resized() {
        gain_bound_ = getLocalBounds().toFloat();
        scale_bound_ = gain_bound_.removeFromRight(gain_bound_.getWidth() * .5f);
        const auto bound = getLocalBounds().toFloat();
        background_path_.clear();
        background_path_.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                           ui_base_.getFontSize() * .5f, ui_base_.getFontSize() * .5f,
                                           false, false, true, true);
    }

    void OutputValuePanel::updateGainValue() {
        const auto new_gain = static_cast<float>(processor_ref_.getController().getGainCompensation());
        const auto new_scale = scale_.load();
        const auto new_learning = lm_para_->getValue() > .5f;
        if (std::abs(new_gain - c_gain_) > 0.0 || std::abs(new_scale - c_scale_) > 0.0 ||
            new_learning != c_learning_) {
            c_gain_ = new_gain;
            c_scale_ = new_scale;
            c_learning_ = new_learning;
            if (!c_learning_) {
                if (c_gain_ <= 0.04) {
                    gain_string_ = juce::String(c_gain_, 1, false);
                } else {
                    gain_string_ = "+" + juce::String(c_gain_, 1, false);
                }
            } else {
                gain_string_ = "L";
            }
            scale_string_ = juce::String(static_cast<int>(std::round(scale_.load()))) + "%";
            repaint();
        }
    }

    void OutputValuePanel::valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                                    const juce::Identifier &property) {
        juce::ignoreUnused(tree_whose_property_has_changed);
        if (zlgui::UIBase::isBoxProperty(zlgui::BoxIdx::kOutputBox, property)) {
            repaint();
        }
    }
} // zlpanel
