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

#include "../interface_definitions.hpp"

namespace zlgui {
    class CallOutBoxLAF final : public juce::LookAndFeel_V4 {
    public:
        explicit CallOutBoxLAF(UIBase &base) : ui_base_(base) {
        }

        void drawCallOutBoxBackground(juce::CallOutBox &box, juce::Graphics &g,
                                      const juce::Path &,
                                      juce::Image &) override {
            g.setColour(ui_base_.getBackgroundColor());
            g.fillRoundedRectangle(box.getLocalBounds().toFloat(), ui_base_.getFontSize() * .5f);
        }

        int getCallOutBoxBorderSize(const juce::CallOutBox &) override {
            return 0;
        }

        float getCallOutBoxCornerSize(const juce::CallOutBox &) override {
            return 0.f;
        }

    private:
        UIBase &ui_base_;
    };
}
