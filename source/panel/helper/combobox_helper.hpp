// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstddef>

#include "../../gui/combobox/compact_combobox/compact_combobox_look_and_feel.hpp"
#include "../../state/state_definitions.hpp"

namespace zlpanel::combobox_helper {
    [[nodiscard]] inline constexpr zlgui::combobox::Alignment getAlignment(const std::size_t alignment) noexcept {
        switch (alignment) {
        case zlstate::PComboboxAlignment::kLeft:
            return zlgui::combobox::Alignment::kLeft;
        case zlstate::PComboboxAlignment::kRight:
            return zlgui::combobox::Alignment::kRight;
        case zlstate::PComboboxAlignment::kCenter:
        default:
            return zlgui::combobox::Alignment::kCenter;
        }
    }

    [[nodiscard]] inline constexpr zlgui::combobox::Alignment getPaddedAlignment(const std::size_t alignment) noexcept {
        switch (alignment) {
        case zlstate::PComboboxAlignment::kLeft:
            return zlgui::combobox::Alignment::kLeftPadding;
        case zlstate::PComboboxAlignment::kRight:
            return zlgui::combobox::Alignment::kRightPadding;
        case zlstate::PComboboxAlignment::kCenter:
        default:
            return zlgui::combobox::Alignment::kCenter;
        }
    }
}
