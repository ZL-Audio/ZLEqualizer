// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

namespace zlpanel {
    SoloPanel::SoloPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base) {
        base_.setSoloWholeIdx(2 * zlp::kBandNum);
        base_.getSoloWholeIdxTree().addListener(this);
        setInterceptsMouseClicks(false, false);
    }

    SoloPanel::~SoloPanel() {
        p_ref_.getController().setSoloWholeIdx(2 * zlp::kBandNum);
        base_.getSoloWholeIdxTree().removeListener(this);
    }

    void SoloPanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getBackgroundColour().withAlpha(.625f));
        if (x_left_ > 0.f) {
            g.fillRect(juce::Rectangle<float>{0.f, 0.f, x_left_, bound.getHeight()});
        }
        if (x_right_ < bound.getWidth()) {
            g.fillRect(juce::Rectangle<float>{x_right_, 0.f, bound.getWidth() - x_right_, bound.getHeight()});
        }
    }

    bool SoloPanel::isSoloSide() const {
        return c_solo_side_;
    }

    void SoloPanel::updateX(const float x_left, const float x_right) {
        x_left_ = x_left;
        x_right_ = x_right;
    }

    void SoloPanel::updateBand() const {
        base_.setSoloWholeIdx(2 * zlp::kBandNum);
    }

    void SoloPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) {
        const auto solo_whole_idx = base_.getSoloWholeIdx();
        p_ref_.getController().setSoloWholeIdx(solo_whole_idx);
        if (solo_whole_idx == 2 * zlp::kBandNum) {
            setVisible(false);
            x_left_ = 0.f;
            x_right_ = 1e15f;
        } else {
            setVisible(true);
            if (solo_whole_idx < zlp::kBandNum) {
                c_solo_side_ = false;
            } else {
                c_solo_side_ = true;
            }
        }
    }
}
