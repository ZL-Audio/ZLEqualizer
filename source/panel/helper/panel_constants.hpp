// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

namespace zlpanel {
    inline constexpr float kFontSizeOverWidth = 0.016f;
    inline constexpr float kPaddingScale = .5f;
    inline constexpr float kSliderWidthScale = 6.5f;
    inline constexpr float kSliderHeightScale = 2.8f;
    inline constexpr float kSmallSliderWidthScale = 5.5f;
    inline constexpr float kButtonScale = 2.f;
    inline constexpr float kBoxHeightScale = 1.75f;
    inline constexpr float kDraggerScale = 1.f;
    inline constexpr float kSliderDraggingDistanceScale = 6.5f;
    inline constexpr float kFFTSizeOverWidth = 0.984f;

    inline int getPaddingSize(const float font_size) {
        return static_cast<int>(std::round(font_size * kPaddingScale));
    }

    inline int getSliderWidth(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderWidthScale));
    }

    inline int getSliderHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderHeightScale));
    }

    inline int getSliderDraggingDistance(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderDraggingDistanceScale));
    }

    inline int getButtonSize(const float font_size) {
        return static_cast<int>(std::round(font_size * kButtonScale));
    }

    inline int getBoxHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * kBoxHeightScale));
    }

    inline int getSmallSliderWidth(const float font_size) {
        return static_cast<int>(std::round(font_size * kSmallSliderWidthScale));
    }

    inline float getDraggerSize(const float font_size) {
        return font_size * kDraggerScale;
    }

    inline int getBottomPadding(const float font_size) {
        return static_cast<int>(std::round(font_size * 1.25f));
    }

    inline int getBottomAreaHeight(const float font_size) {
        const auto box_height = getBoxHeight(font_size);
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);
        return 3 * box_height + button_height + 5 * padding + getBottomPadding(font_size);
    }
}
