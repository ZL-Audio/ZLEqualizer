// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"
#include "scale_panel_layout.hpp"

namespace zlpanel {
    ScalePanel::ScalePanel(PluginProcessor& p,
                           zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        scale_label_panel_(p, base, tooltip_helper),
        eq_max_box_({juce::String(static_cast<int>(base_.getCurveDBScale(0))),
                     juce::String(static_cast<int>(base_.getCurveDBScale(1))),
                     juce::String(static_cast<int>(base_.getCurveDBScale(2)))}, base),
        eq_max_attach_(eq_max_box_.getBox(), p.parameters_NA_, zlstate::PEQMaxDB::kID, updater_),
        fft_top_box_(zlstate::PFFTTopDB::kChoices, base),
        fft_top_attach_(fft_top_box_.getBox(), p.parameters_NA_, zlstate::PFFTTopDB::kID, updater_),
        fft_min_box_(zlstate::PFFTMinDB::kChoices, base),
        fft_min_attach_(fft_min_box_.getBox(), p.parameters_NA_, zlstate::PFFTMinDB::kID, updater_) {
        juce::ignoreUnused(tooltip_helper);

        scale_label_panel_.setBufferedToImage(true);
        addAndMakeVisible(scale_label_panel_);

        const auto top_popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards);
        fft_top_box_.setScrollEnabled(true);
        fft_top_box_.setAlpha(kFFTAlpha);
        fft_top_box_.getLAF().setFontScale(1.25f);
        fft_top_box_.getLAF().setOption(top_popup_option);
        fft_top_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        fft_top_box_.getLAF().setAlignLabel(false);
        addAndMakeVisible(fft_top_box_);

        const auto fft_popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);
        fft_min_box_.setScrollEnabled(true);
        fft_min_box_.setAlpha(kFFTAlpha);
        fft_min_box_.getLAF().setFontScale(1.25f);
        fft_min_box_.getLAF().setOption(fft_popup_option);
        fft_min_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        fft_min_box_.getLAF().setAlignLabel(false);
        addAndMakeVisible(fft_min_box_);

        const auto eq_popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards);
        eq_max_box_.setScrollEnabled(true);
        eq_max_box_.getLAF().setFontScale(1.25f);
        eq_max_box_.getLAF().setOption(eq_popup_option);
        eq_max_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        eq_max_box_.getLAF().setAlignLabel(false);
        addAndMakeVisible(eq_max_box_);

        setInterceptsMouseClicks(false, true);
    }

    int ScalePanel::getIdealWidth() const {
        return juce::roundToInt(
            base_.getFontSize() * scale_panel_layout::getContentWidthUnits(true));
    }

    void ScalePanel::resized() {
        scale_label_panel_.setBounds(getLocalBounds());

        const auto bound = getLocalBounds().toFloat();
        const auto layout = scale_panel_layout::getMetrics(
            bound.getWidth(), base_.getFontSize(), use_wide_layout_);
        const auto content_bound = bound.withLeft(bound.getRight() - layout.content_width);
        const auto box_height = static_cast<int>(std::round(base_.getFontSize() * 1.3f));
        const auto eq_box_width = static_cast<int>(layout.eq_column_width);
        const auto fft_top_box_width = static_cast<int>(std::floor(layout.fft_column_width));
        const auto fft_min_box_width = static_cast<int>(std::floor(layout.floor_control_width));
        fft_top_box_.setBounds(0, 0, fft_top_box_width, box_height);
        fft_min_box_.setBounds(0, 0, fft_min_box_width, box_height);
        const auto eq_box_bound = juce::Rectangle<int>(0, 0,
                                                       eq_box_width,
                                                       box_height);
        eq_max_box_.setBounds(eq_box_bound);

        const auto unit_height = getUnitHeight();
        eq_max_box_.setTransform(juce::AffineTransform::translation(
            content_bound.getX(),
            base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
        fft_top_box_.setTransform(juce::AffineTransform::translation(
            content_bound.getRight() - static_cast<float>(fft_top_box_width) - layout.right_padding,
            base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
        fft_min_box_.setTransform(juce::AffineTransform::translation(
            content_bound.getRight() - static_cast<float>(fft_min_box_width) - layout.right_padding,
            6.f * unit_height + base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
    }

    void ScalePanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto fft_top_idx = fft_top_box_.getBox().getSelectedItemIndex();
        updateFFTMinChoices(fft_top_idx);
        const auto fft_min_idx = fft_min_box_.getBox().getSelectedItemIndex();
        updateWideLayout(fft_top_idx, fft_min_idx);
        scale_label_panel_.setScaleIdx(eq_max_box_.getBox().getSelectedItemIndex(),
                                       fft_top_idx,
                                       fft_min_idx);
    }

    float ScalePanel::getUnitHeight() const {
        const auto bound = getLocalBounds().toFloat();
        return (bound.getHeight() - 2.f * base_.getFontSize() * kDraggerScale
            - static_cast<float>(getBottomAreaHeight(base_.getFontSize()))) / 6.f;
    }

    void ScalePanel::updateFFTMinChoices(const int fft_top_idx) {
        if (fft_top_idx < 0 || fft_top_idx == c_fft_top_idx_) {
            return;
        }

        // The stored choice remains a span; show the absolute floor produced by the selected top.
        auto& box = fft_min_box_.getBox();
        const auto selected_idx = box.getSelectedItemIndex();
        const auto top_db = zlstate::PFFTTopDB::kDBs[static_cast<size_t>(fft_top_idx)];
        for (size_t i = 0; i < zlstate::PFFTMinDB::kDBs.size(); ++i) {
            const auto floor_db = static_cast<int>(std::round(top_db + zlstate::PFFTMinDB::kDBs[i]));
            box.changeItemText(static_cast<int>(i + 1), juce::String(floor_db));
        }
        box.setSelectedItemIndex(selected_idx, juce::dontSendNotification);
        fft_min_box_.repaint();
        c_fft_top_idx_ = fft_top_idx;
    }

    void ScalePanel::updateWideLayout(const int fft_top_idx, const int fft_min_idx) {
        if (fft_top_idx < 0 || fft_min_idx < 0) {
            return;
        }

        const auto fft_top = zlstate::PFFTTopDB::kDBs[static_cast<size_t>(fft_top_idx)];
        const auto fft_range = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(fft_min_idx)];
        const auto should_use_wide_layout = scale_panel_layout::usesThreeDigitFloor(
            fft_top, fft_range);
        if (should_use_wide_layout != use_wide_layout_) {
            use_wide_layout_ = should_use_wide_layout;
            resized();
        }
    }
}
