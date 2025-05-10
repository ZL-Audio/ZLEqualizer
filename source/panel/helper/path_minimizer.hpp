// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlpanel {
    class PathMinimizer {
    public:
        static constexpr float kTol = 0.01f;

        explicit PathMinimizer(juce::Path &path) : path_ref_(path) {
        }

        template<bool start = true>
        void startNewSubPath(const float x, const float y) {
            if (start) {
                path_ref_.startNewSubPath(x, y);
            } else {
                path_ref_.lineTo(x, y);
            }
            start_x_ = x;
            start_y_ = y;
            current_x_ = x;
            current_y_ = y;
        }

        void lineTo(const float x, const float y) {
            const auto w = (current_x_ - start_x_) / (x - start_x_);
            if (std::abs(w * start_y_ + (1.f - w) * y - current_y_) > kTol) {
                path_ref_.lineTo(current_x_, current_y_);
                start_x_ = x;
                start_y_ = y;
            }
            current_x_ = x;
            current_y_ = y;
        }

        void finish() const {
            path_ref_.lineTo(current_x_, current_y_);
        }

    private:
        juce::Path &path_ref_;
        float start_x_{0.}, start_y_{0.};
        float current_x_{0.}, current_y_{0.};
    };
}
