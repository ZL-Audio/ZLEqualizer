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

namespace zlgui {
    class SnappingSlider final : public juce::Slider {
    public:
        explicit SnappingSlider(UIBase &base, const juce::String &name = "") : juce::Slider(name), ui_base_(base) {
        }

        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override {
            juce::MouseWheelDetails w = wheel;
            w.deltaX *= ui_base_.getSensitivity(kMouseWheel);
            w.deltaY *= ui_base_.getSensitivity(kMouseWheel);
            if (e.mods.isShiftDown()) {
                const auto dir = ui_base_.getIsMouseWheelShiftReverse() ? -1.f : 1.f;
                w.deltaX *= ui_base_.getSensitivity(kMouseWheelFine) * dir;
                w.deltaY *= ui_base_.getSensitivity(kMouseWheelFine) * dir;
            }
            Slider::mouseWheelMove(e, w);
        }

    private:
        UIBase &ui_base_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnappingSlider)
    };
}
