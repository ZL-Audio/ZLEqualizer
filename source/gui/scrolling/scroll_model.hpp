// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

#include <juce_core/juce_core.h>

namespace zlgui::scrolling {
    class ScrollModel final {
    public:
        [[nodiscard]] double getPosition() const {
            return position_;
        }

        [[nodiscard]] double getTargetPosition() const {
            return target_position_;
        }

        bool setPosition(const double position, const double maximum) {
            const auto next_position = clamp(position, maximum);
            target_position_ = next_position;
            update_pending_ = false;
            if (std::abs(next_position - position_) <= kPositionEpsilon) {
                position_ = next_position;
                return false;
            }
            position_ = next_position;
            return true;
        }

        void requestPosition(const double position, const double maximum) {
            target_position_ = clamp(position, maximum);
            update_pending_ = std::abs(target_position_ - position_) > kPositionEpsilon;
        }

        bool flush(const double maximum) {
            if (!update_pending_) {
                return false;
            }
            update_pending_ = false;
            const auto next_position = clamp(target_position_, maximum);
            target_position_ = next_position;
            if (std::abs(next_position - position_) <= kPositionEpsilon) {
                position_ = next_position;
                return false;
            }
            position_ = next_position;
            return true;
        }

        bool clampToMaximum(const double maximum) {
            const auto next_position = clamp(position_, maximum);
            const auto position_changed = std::abs(next_position - position_) > kPositionEpsilon;
            position_ = next_position;
            target_position_ = clamp(target_position_, maximum);
            update_pending_ = std::abs(target_position_ - position_) > kPositionEpsilon;
            return position_changed;
        }

        void reset(const double position = 0.0) {
            position_ = juce::jmax(0.0, position);
            target_position_ = position_;
            update_pending_ = false;
        }

    private:
        static constexpr double kPositionEpsilon = .01;

        double position_{0.0};
        double target_position_{0.0};
        bool update_pending_{false};

        static double clamp(const double position, const double maximum) {
            return juce::jlimit(0.0, juce::jmax(0.0, maximum), position);
        }
    };
}
