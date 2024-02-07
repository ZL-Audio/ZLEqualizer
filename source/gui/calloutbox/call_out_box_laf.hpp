// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_CALLOUTBOXLAF_HPP
#define ZL_CALLOUTBOXLAF_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"

namespace zlInterface {
    class CallOutBoxLAF final : public juce::LookAndFeel_V4 {
    public:
        explicit CallOutBoxLAF(UIBase &base) : uiBase(base) {
        }

        void drawCallOutBoxBackground(juce::CallOutBox &box, juce::Graphics &g,
                                      const juce::Path &,
                                      juce::Image &) override {
            // g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
            // g.fillRoundedRectangle(box.getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
            // g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
            g.setColour(uiBase.getBackgroundColor());
            g.fillRoundedRectangle(box.getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        }

        int getCallOutBoxBorderSize(const juce::CallOutBox &) override {
            return 0;
        }

        float getCallOutBoxCornerSize(const juce::CallOutBox &) override {
            return 0.f;
        }

    private:
        UIBase &uiBase;
    };
}
#endif //ZL_CALLOUTBOXLAF_HPP
