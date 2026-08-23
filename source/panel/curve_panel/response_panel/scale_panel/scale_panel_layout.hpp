// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cmath>

namespace zlpanel::scale_panel_layout {
    inline constexpr float kNarrowContentWidthUnits{4.f};

    inline constexpr float kThreeDigitExtraWidthUnits{.5f};

    inline constexpr float kEQColumnWidthUnits{2.f};
    inline constexpr float kNarrowFloorControlWidthUnits{3.f};
    inline constexpr float kRightPaddingUnits{.1f};

    struct Metrics {
        float content_width;
        float eq_column_width;
        float fft_column_width;
        float floor_control_width;
        float right_padding;
    };

    [[nodiscard]] constexpr bool usesThreeDigitFloor(const float fft_top,
                                                     const float fft_range) noexcept {
        return fft_top + fft_range <= -100.f;
    }

    [[nodiscard]] constexpr float getContentWidthUnits(const bool use_three_digit_floor) noexcept {
        return kNarrowContentWidthUnits
            + (use_three_digit_floor ? kThreeDigitExtraWidthUnits : 0.f);
    }

    [[nodiscard]] inline Metrics getMetrics(const float available_width,
                                            const float font_size,
                                            const bool use_three_digit_floor) noexcept {
        const auto content_width = std::min(
            available_width, font_size * getContentWidthUnits(use_three_digit_floor));
        const auto eq_column_width = std::floor(std::min(
            content_width, font_size * kEQColumnWidthUnits));
        const auto right_padding = std::min(
            font_size * kRightPaddingUnits, std::max(0.f, content_width - eq_column_width));
        const auto fft_column_width = std::max(
            0.f, content_width - eq_column_width - right_padding);
        const auto floor_control_width = use_three_digit_floor
            ? fft_column_width
            : std::min(content_width, font_size * kNarrowFloorControlWidthUnits);

        return {content_width, eq_column_width, fft_column_width,
                floor_control_width, right_padding};
    }
}
