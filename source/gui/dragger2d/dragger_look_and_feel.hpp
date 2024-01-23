// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLTest_DRAGGER_LOOK_AND_FEEL_HPP
#define ZLTest_DRAGGER_LOOK_AND_FEEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"

namespace zlInterface {
    class DraggerLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit DraggerLookAndFeel(UIBase &base) : uiBase(base) {}

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            if (!active.load()) { return; }
            auto bound = button.getLocalBounds().toFloat();
            auto radius = std::min(bound.getHeight(), bound.getWidth());
            bound = bound.withSizeKeepingCentre(radius, radius);
            if (shouldDrawButtonAsDown || button.getToggleState()) {
                g.setColour(uiBase.getTextColor());
                g.fillEllipse(bound);
            } else if (shouldDrawButtonAsHighlighted) {
                g.setColour(uiBase.getTextColor().withMultipliedAlpha(0.5f));
                g.fillEllipse(bound);
            }
            radius *= .75f;
            bound = bound.withSizeKeepingCentre(radius, radius);
            g.setColour(colour);
            g.fillEllipse(bound);
        }

        inline void setColour(const juce::Colour c) { colour = c; }

        void setActive(const bool f) { active.store(f); }

    private:
        juce::Colour colour;
        std::atomic<bool> active {true};
        UIBase &uiBase;
    };
}

#endif //ZLTest_DRAGGER_LOOK_AND_FEEL_HPP
