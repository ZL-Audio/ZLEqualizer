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

namespace zlInterface {
    class TooltipLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit TooltipLookAndFeel(UIBase &base) : uiBase(base) {
        }

        juce::Rectangle<int> getTooltipBounds(const juce::String &tipText,
                                              const juce::Point<int> screenPos,
                                              const juce::Rectangle<int> parentArea) override {
            const auto tl = getTipTextLayout(tipText,
                                             static_cast<float>(parentArea.getWidth()) * .4f,
                                             static_cast<float>(parentArea.getHeight()) * .4f);
            const auto w = static_cast<int>(std::ceil(tl.getWidth() + uiBase.getFontSize() * .25f));
            const auto h = static_cast<int>(std::ceil(tl.getHeight() + uiBase.getFontSize() * .25f));
            const auto padding = static_cast<int>(std::round(uiBase.getFontSize() * .5f));
            if (screenPos.x > parentArea.getCentreX() && screenPos.y < parentArea.getCentreY()) {
                return juce::Rectangle<int>(parentArea.getX() + padding,
                                            parentArea.getY() + padding,
                                            w, h);
            } else {
                return juce::Rectangle<int>(parentArea.getRight() - w - padding,
                                            parentArea.getY() + padding,
                                            w, h);
            }
        }

        void drawTooltip(juce::Graphics &g, const juce::String &text, const int width, const int height) override {
            const juce::Rectangle<float> bound{static_cast<float>(width), static_cast<float>(height)};

            g.setColour(uiBase.getBackgroundColor().withAlpha(.875f));
            g.fillRect(bound);

            const auto tl = getTipTextLayout(text, bound.getWidth(), bound.getHeight());
            tl.draw(g, bound);
        }

    private:
        UIBase &uiBase;

        juce::TextLayout getTipTextLayout(const juce::String &text,
                                          const float w, const float h) const {
            juce::AttributedString s;
            s.setJustification(juce::Justification::centredLeft);
            s.append(text, juce::FontOptions(uiBase.getFontSize() * 1.5f), uiBase.getTextColor());
            juce::TextLayout tl;
            tl.createLayout(s, w, h);
            return tl;
        }
    };
}
