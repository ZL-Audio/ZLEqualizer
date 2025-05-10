// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "loudness_display.hpp"

namespace zlpanel {
    LoudnessDisplay::LoudnessDisplay(PluginProcessor &p, zlgui::UIBase &base)
        : processor_ref_(p), ui_base_(base) {
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            is_threshold_auto_paras_[i] = processor_ref_.parameters_.getParameter(zlp::dynamicLearn::ID + suffix);
            is_dynamic_on_paras_[i] = processor_ref_.parameters_.getParameter(zlp::dynamicON::ID + suffix);
        }
        band_idx_para_ = processor_ref_.parameters_NA_.getParameter(zlstate::selectedBandIdx::ID);
        lookAndFeelChanged();
    }

    void LoudnessDisplay::paint(juce::Graphics &g) {
        const auto loudness = processor_ref_.getController().getSideLoudness(band_idx_);
        const auto p = 1. + std::clamp(loudness, -80.0, 0.0) / 80;
        auto bound = getLocalBounds().toFloat();
        bound = bound.withWidth(bound.getWidth() * static_cast<float>(p));
        g.setColour(colour_);
        g.fillRect(bound);
    }

    void LoudnessDisplay::checkVisible() {
        band_idx_ = static_cast<size_t>(band_idx_para_->convertFrom0to1(band_idx_para_->getValue()));
        const auto f = (is_threshold_auto_paras_[band_idx_]->getValue() < 0.5f)
                              && (is_dynamic_on_paras_[band_idx_]->getValue() > 0.5f);
        setVisible(f && should_visible_ && colour_.getFloatAlpha() > 0.005f);
    }

    void LoudnessDisplay::lookAndFeelChanged() {
        colour_ = ui_base_.getColourByIdx(zlgui::kSideLoudnessColour);
    }

    void LoudnessDisplay::updateVisible(const bool x) {
        should_visible_ = x;
    }
} // zlpanel
