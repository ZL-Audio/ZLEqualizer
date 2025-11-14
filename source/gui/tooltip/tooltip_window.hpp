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

namespace zlgui::tooltip {
    class TooltipWindow final : public juce::TooltipWindow {
    public:
        explicit TooltipWindow(Component* parent_component, const int milliseconds_before_appears = 700)
            : juce::TooltipWindow(parent_component, milliseconds_before_appears) {
            setInterceptsMouseClicks(false, false);
        }

        juce::String getTipFor(Component& c) override {
            return juce::TooltipWindow::getTipFor(c);
        }
    };
}
