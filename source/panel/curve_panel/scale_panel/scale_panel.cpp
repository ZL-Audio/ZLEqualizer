// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"

namespace zlpanel {
    ScalePanel::ScalePanel(PluginProcessor& p,
                           zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        eq_max_box_(zlstate::PEQMaxDB::kChoices, base),
        eq_max_attach_(eq_max_box_.getBox(), p.parameters_NA_, zlstate::PEQMaxDB::kID, updater_),
        fft_min_box_(zlstate::PFFTMinDB::kChoices, base),
        fft_min_attach_(fft_min_box_.getBox(), p.parameters_NA_, zlstate::PFFTMinDB::kID, updater_) {
        juce::ignoreUnused(tooltip_helper);
        setInterceptsMouseClicks(false, true);

        const auto fft_popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);
        fft_min_box_.setAlpha(kFFTAlpha);
        fft_min_box_.getLAF().setFontScale(1.25f);
        fft_min_box_.getLAF().setOption(fft_popup_option);
        fft_min_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        addAndMakeVisible(fft_min_box_);

        const auto eq_popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards);
        eq_max_box_.getLAF().setFontScale(1.25f);
        eq_max_box_.getLAF().setOption(eq_popup_option);
        eq_max_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        addAndMakeVisible(eq_max_box_);

        setInterceptsMouseClicks(false, true);
    }

    void ScalePanel::paint(juce::Graphics& g) {
        if (c_eq_max_idx_ < 0 || c_fft_min_idx_ < 0) {
            return;
        }
        const auto bound = getLocalBounds().toFloat();
        const auto unit_height = getUnitHeight();

        const float label_height = base_.getFontSize() * 1.1f;
        const float y0 = base_.getFontSize() * kDraggerScale * .5f - label_height * .5f;

        const auto eq_unit = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(c_eq_max_idx_)] / 3.f;
        const auto fft_unit = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(c_fft_min_idx_)] / 6.f;
        g.setFont(base_.getFontSize() * 1.25f);

        const auto db_label_bound = juce::Rectangle<float>(bound.getWidth() * .5f + base_.getFontSize() * .2f, y0,
                                                           bound.getWidth(), label_height);
        g.setColour(base_.getTextColor());
        g.drawText("dB", db_label_bound, juce::Justification::centredLeft);
        // draw eq labels and fft labels
        for (int i = 1; i < 7; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                0.f, y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            fft_label_bound.removeFromRight(base_.getFontSize() * .1f);
            auto eq_label_bound = fft_label_bound.removeFromLeft(fft_label_bound.getWidth() * .5f);
            if (i != 6) {
                const auto fft_value = std::round(static_cast<float>(i) * fft_unit);
                g.setColour(base_.getTextColor().withAlpha(kFFTAlpha));
                g.drawText(juce::String(fft_value), fft_label_bound, juce::Justification::centredRight);
            }
            const auto eq_value = std::round((3.f - static_cast<float>(i)) * eq_unit);
            g.setColour(base_.getTextColor());
            g.drawText(juce::String(eq_value), eq_label_bound, juce::Justification::centredRight);
        }
        // draw the remaining fft labels
        for (int i = 7; i < 12; ++i) {
            auto fft_label_bound = juce::Rectangle<float>(
                0.f, y0 + static_cast<float>(i) * unit_height, bound.getWidth(), label_height);
            if (fft_label_bound.getBottom() > bound.getHeight() - base_.getFontSize() * 3.f) {
                break;
            }
            fft_label_bound.removeFromRight(base_.getFontSize() * .1f);
            const auto fft_value = std::round(static_cast<float>(i) * fft_unit);
            g.setColour(base_.getTextColor().withAlpha(kFFTAlpha));
            g.drawText(juce::String(fft_value), fft_label_bound, juce::Justification::centredRight);
        }
    }

    int ScalePanel::getIdealWidth() const {
        return juce::roundToInt(base_.getFontSize() * 3.5f);
    }

    void ScalePanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        const auto box_height = static_cast<int>(std::round(base_.getFontSize() * 1.3f));
        const auto fft_box_bound = juce::Rectangle<int>(0, 0,
                                                        static_cast<int>(std::floor(bound.getWidth() * .75f)),
                                                        box_height);
        fft_min_box_.setBounds(fft_box_bound);
        const auto eq_box_bound = juce::Rectangle<int>(0, 0,
                                                       static_cast<int>(std::floor(bound.getWidth() * .5f)),
                                                       box_height);
        eq_max_box_.setBounds(eq_box_bound);

        const auto unit_height = getUnitHeight();
        eq_max_box_.setTransform(juce::AffineTransform::translation(
            .5f * bound.getWidth() - static_cast<float>(eq_box_bound.getWidth()),
            .5f * base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
        fft_min_box_.setTransform(juce::AffineTransform::translation(
            bound.getWidth() - static_cast<float>(fft_box_bound.getWidth()) - base_.getFontSize() * .1f,
            6.f * unit_height + .5f * base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
    }

    void ScalePanel::repaintCallBackSlow() {
        updater_.updateComponents();
        if (c_eq_max_idx_ != eq_max_box_.getBox().getSelectedItemIndex()
            || c_fft_min_idx_ != fft_min_box_.getBox().getSelectedItemIndex()) {
            c_eq_max_idx_ = eq_max_box_.getBox().getSelectedItemIndex();
            c_fft_min_idx_ = fft_min_box_.getBox().getSelectedItemIndex();
            repaint();
        }
    }

    float ScalePanel::getUnitHeight() const {
        const auto bound = getLocalBounds().toFloat();
        const auto box_height = juce::roundToInt(base_.getFontSize() * kBoxHeightScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        return (bound.getHeight() - 2.f * base_.getFontSize() * kDraggerScale
            - static_cast<float>(3 * box_height + button_height + 7 * padding)) / 6.f;
    }
}
