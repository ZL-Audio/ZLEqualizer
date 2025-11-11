// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "side_loudness_display_panel.hpp"

namespace zlpanel {
    SideLoudnessDisplayPanel::SideLoudnessDisplayPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void SideLoudnessDisplayPanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        const auto loudness_p = std::clamp(loudness_ * .0125f + 1.f, 0.f, 1.f);
        g.setGradientFill(gradient_);
        g.fillRect(bound.withWidth(bound.getWidth() * loudness_p));
    }

    void SideLoudnessDisplayPanel::resized() {
        lookAndFeelChanged();
    }

    void SideLoudnessDisplayPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            dynamic_on_ = p_ref_.parameters_.getRawParameterValue(zlp::PDynamicON::kID + band_s);
            dynamic_learn_on_ = p_ref_.parameters_.getRawParameterValue(zlp::PDynamicLearn::kID + band_s);
        } else {
            dynamic_on_ = nullptr;
            dynamic_learn_on_ = nullptr;
        }
    }

    void SideLoudnessDisplayPanel::repaintCallBack() {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum && isVisible()) {
            loudness_ = static_cast<float>(p_ref_.getController().getSideLoudness(band));
        }
    }

    void SideLoudnessDisplayPanel::repaintCallBackSlow() {
        if (dynamic_on_ != nullptr) {
            if (dynamic_on_->load(std::memory_order::relaxed) > .5f
                && dynamic_learn_on_->load(std::memory_order::relaxed) < .5f) {
                setVisible(true);
            } else {
                setVisible(false);
            }
        } else {
            setVisible(false);
        }
    }

    void SideLoudnessDisplayPanel::lookAndFeelChanged() {
        const auto bound = getLocalBounds().toFloat();
        gradient_.clearColours();
        gradient_.point1 = {0.f, 0.f};
        gradient_.point2 = {bound.getWidth(), 0.f};
        const auto colour = base_.getColourBlendedWithBackground(base_.getTextColour(), .5f);
        gradient_.addColour(0.f, colour.withAlpha(0.f));
        gradient_.addColour(.1f, colour);
        gradient_.addColour(1.f, colour);
    }
}
