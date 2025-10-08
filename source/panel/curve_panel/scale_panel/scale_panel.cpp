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
        scale_label_panel_(p, base, tooltip_helper),
        eq_max_box_(zlstate::PEQMaxDB::kChoices, base),
        eq_max_attach_(eq_max_box_.getBox(), p.parameters_NA_, zlstate::PEQMaxDB::kID, updater_),
        fft_min_box_(zlstate::PFFTMinDB::kChoices, base),
        fft_min_attach_(fft_min_box_.getBox(), p.parameters_NA_, zlstate::PFFTMinDB::kID, updater_) {
        juce::ignoreUnused(tooltip_helper);

        scale_label_panel_.setBufferedToImage(true);
        addAndMakeVisible(scale_label_panel_);

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

    int ScalePanel::getIdealWidth() const {
        return juce::roundToInt(base_.getFontSize() * 3.5f);
    }

    void ScalePanel::resized() {
        scale_label_panel_.setBounds(getLocalBounds());

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
            base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
        fft_min_box_.setTransform(juce::AffineTransform::translation(
            bound.getWidth() - static_cast<float>(fft_box_bound.getWidth()) - base_.getFontSize() * .1f,
            6.f * unit_height + base_.getFontSize() * kDraggerScale - .5f * static_cast<float>(box_height)));
    }

    void ScalePanel::repaintCallBackSlow() {
        updater_.updateComponents();
        scale_label_panel_.setMaxIdx(eq_max_box_.getBox().getSelectedItemIndex(),
                                     fft_min_box_.getBox().getSelectedItemIndex());
    }

    float ScalePanel::getUnitHeight() const {
        const auto bound = getLocalBounds().toFloat();
        return (bound.getHeight() - 2.f * base_.getFontSize() * kDraggerScale
            - static_cast<float>(getBottomAreaHeight(base_.getFontSize()))) / 6.f;
    }
}
