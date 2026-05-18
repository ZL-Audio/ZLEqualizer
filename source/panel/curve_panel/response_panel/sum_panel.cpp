// Copyright (C) 2026 - zsliu98
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
                       std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum>& dynamic_mags) {
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

        {
            namespace hn = hwy::HWY_NAMESPACE;
            static constexpr hn::ScalableTag<float> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            const auto vk = hn::Set(d, k);
            const auto vb = hn::Set(d, b);
            size_t i = 0;
            for (; i + lanes <= temp_db_.size(); i += lanes) {
                auto v_sum = hn::Zero(d);
                for (const size_t on_index : on_indices) {
                    const float* mag_ptr = dynamic_mags[on_index].data();
                    v_sum = hn::Add(v_sum, hn::Load(d, mag_ptr + i));
                }
                hn::Store(hn::MulAdd(vk, v_sum, vb), d, temp_db_.data() + i);
            }
            for (; i < temp_db_.size(); ++i) {
                float sum = 0.0f;
                for (const size_t on_index : on_indices) {
                    sum += dynamic_mags[on_index][i];
                }
                temp_db_[i] = std::fma(k, sum, b);
            }
        }

        PathMinimizer<1> minimizer(path);
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
        curve_thickness_ = base_.getFontSize() * .275f * base_.getSumEQCurveThickness();
    }
}
