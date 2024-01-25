// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CLICK_BUTTON_LOOK_AND_FEEL_HPP
#define ZLEqualizer_CLICK_BUTTON_LOOK_AND_FEEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class ClickButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit ClickButtonLookAndFeel(juce::Drawable *image, UIBase &base) : drawable(image), uiBase(base) {}

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button, bool shouldDrawButtonAsHighlighted,
                                bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button);
            auto bound = button.getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() - padding.load(),
                                                bound.getHeight() - padding.load());
            if (drawable != nullptr) {
                const auto tempDrawable = drawable->createCopy();
                tempDrawable->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
                if (editable.load()) {
                    if (shouldDrawButtonAsDown) {
                        tempDrawable->drawWithin(g, bound, juce::RectanglePlacement::centred, 1.f);
                    } else if (shouldDrawButtonAsHighlighted) {
                        tempDrawable->drawWithin(g, bound, juce::RectanglePlacement::centred, .75f);
                    } else {
                        tempDrawable->drawWithin(g, bound, juce::RectanglePlacement::centred, .5f);
                    }
                } else {
                    tempDrawable->drawWithin(g, bound, juce::RectanglePlacement::centred, .25f);
                }
            }
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline void setCornerSize(const float x) { cornerSize.store(x); }

        inline void setPadding(const float x) { padding.store(x); }

        inline void setCurve(const bool tl, const bool tr, const bool bl, const bool br) {
            curveTL.store(tl);
            curveTR.store(tr);
            curveBL.store(bl);
            curveBR.store(br);
        }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> cornerSize = 0.f, padding = 0.f;
        std::atomic<bool> curveTL = false, curveTR = false, curveBL = false, curveBR = false;
        juce::Drawable *drawable;

        UIBase &uiBase;
    };
}

#endif //ZLEqualizer_CLICK_BUTTON_LOOK_AND_FEEL_HPP
