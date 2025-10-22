// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlpanel {
    SumPanel::SumPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base) {
        juce::ignoreUnused(p_ref_);
        temp_db_.resize(kNumPoints);
        setInterceptsMouseClicks(false, false);
    }

    void SumPanel::paintSameStereo(juce::Graphics& g) const {
        for (size_t lr = 0; lr < 5; ++lr) {
            const auto i = static_cast<size_t>(4) - lr;
            if (!paths_[i].isEmpty() && is_same_stereo_[i]) {
                g.setColour(base_.getColourMap2(i));
                g.strokePath(paths_[i], juce::PathStrokeType(curve_thickness_,
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::butt));
            }
        }
    }

    void SumPanel::paintDifferentStereo(juce::Graphics& g) const {
        for (size_t lr = 0; lr < 5; ++lr) {
            const auto i = static_cast<size_t>(4) - lr;
            if (!paths_[i].isEmpty() && !is_same_stereo_[i]) {
                g.setColour(base_.getColourMap2(i).withAlpha(kDiffStereoAlphaMultiplier));
                g.strokePath(paths_[i], juce::PathStrokeType(curve_thickness_,
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::butt));
            }
        }
    }

    void SumPanel::resized() {
        lookAndFeelChanged();
    }

    void SumPanel::run(const size_t lr, const bool to_update, const bool is_not_off,
                       const std::span<size_t> on_indices,
                       const std::span<float> xs, float k, float b,
                       std::array<kfr::univector<float>, zlp::kBandNum>& dynamic_mags) {
        if (!to_update) {
            return;
        }

        auto& path{next_paths_[lr]};
        path.clear();

        if (on_indices.empty()) {
            if (is_not_off) {
                path.startNewSubPath(xs[0], b);
                path.lineTo(xs.back(), b);
            }
            return;
        }

        temp_db_ = dynamic_mags[on_indices[0]];
        for (size_t i = 1; i < on_indices.size(); ++i) {
            temp_db_ += dynamic_mags[on_indices[i]];
        }
        temp_db_ = k * temp_db_ + b;

        PathMinimizer minimizer(path);
        minimizer.drawPath<true, false>(xs, std::span(temp_db_));
    }

    void SumPanel::runUpdate(std::array<bool, 5>& to_update_flags_) {
        for (size_t lr = 0; lr < 5; ++lr) {
            if (std::exchange(to_update_flags_[lr], false)) {
                paths_[lr] = next_paths_[lr];
            }
        }
    }

    void SumPanel::updateDrawingParas(const int lr, const bool is_same_stereo) {
        is_same_stereo_[static_cast<size_t>(lr)] = is_same_stereo;
    }

    void SumPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .25f * base_.getEQCurveThickness();
    }
}
