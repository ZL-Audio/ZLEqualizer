// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_graphics/juce_graphics.h>
#include <span>
#include <cmath>
#include <algorithm>

namespace zlpanel {
    template <int kIntTol = 1>
    class GradientMinimizer {
    public:
        static constexpr float kTol = 1e-3f * static_cast<float>(kIntTol);
        static constexpr float kZeroTol = 1e-3f;

        GradientMinimizer(juce::ColourGradient& gradient, juce::Colour colour)
            : gradient_ref_(gradient), colour_(colour) {
        }

        void start(const float proportion, const float alpha) {
            gradient_ref_.addColour(proportion, colour_.withAlpha(alpha));
            start_prop_ = proportion;
            start_alpha_ = alpha;
            current_prop_ = proportion;
            current_alpha_ = alpha;
        }

        void addColour(const float proportion, const float alpha) {
            if (proportion - start_prop_ > 1e-6f) {
                const bool is_zero_portion = (start_alpha_ < kZeroTol &&
                                              current_alpha_ < kZeroTol &&
                                              alpha < kZeroTol);
                if (!is_zero_portion) {
                    const auto w = (current_prop_ - start_prop_) / (proportion - start_prop_);
                    const auto expected_alpha = (1.f - w) * start_alpha_ + w * alpha;
                    if (std::abs(expected_alpha - current_alpha_) > kTol ||
                        (current_alpha_ > kZeroTol && alpha < kZeroTol) ||
                        (current_alpha_ < kZeroTol && alpha > kZeroTol) ||
                        (current_alpha_ > start_alpha_ && current_alpha_ > alpha)) {
                        gradient_ref_.addColour(current_prop_, colour_.withAlpha(current_alpha_));
                        start_prop_ = current_prop_;
                        start_alpha_ = current_alpha_;
                    }
                }
            }
            current_prop_ = proportion;
            current_alpha_ = alpha;
        }

        void finish() {
            gradient_ref_.addColour(current_prop_, colour_.withAlpha(current_alpha_));
        }

        void buildGradient(std::span<const float> proportions, std::span<const float> alphas) {
            if (proportions.empty()) return;
            start(proportions.front(), alphas.front());
            for (size_t i = 1; i < proportions.size(); ++i) {
                addColour(proportions[i], alphas[i]);
            }
            finish();
        }

    private:
        juce::ColourGradient& gradient_ref_;
        juce::Colour colour_;

        float start_prop_{0.f}, start_alpha_{0.f};
        float current_prop_{0.f}, current_alpha_{0.f};
    };
}
