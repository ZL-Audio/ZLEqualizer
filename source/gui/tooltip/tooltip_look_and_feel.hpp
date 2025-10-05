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

namespace zlgui::tooltip {
    class TooltipLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit TooltipLookAndFeel(UIBase& base) : base_(base) {
        }

        juce::Rectangle<int> getTooltipBounds(const juce::String& tip_text,
                                              const juce::Point<int> screen_pos,
                                              const juce::Rectangle<int> parent_area) override {
            const auto tl = getTipTextLayout(tip_text,
                                             static_cast<float>(parent_area.getWidth()) * .4f,
                                             static_cast<float>(parent_area.getHeight()) * .4f);
            const auto w = static_cast<int>(std::ceil(tl.getWidth() + base_.getFontSize() * .25f));
            const auto h = static_cast<int>(std::ceil(tl.getHeight() + base_.getFontSize() * .25f));
            const auto padding = static_cast<int>(std::round(base_.getFontSize() * kPaddingScale));
            if (screen_pos.x > parent_area.getCentreX() && screen_pos.y < parent_area.getCentreY()) {
                return {
                    parent_area.getX(),
                    parent_area.getY(),
                    w + 2 * padding, h + 2 * padding
                };
            } else {
                return {
                    parent_area.getRight() - w - 2 * padding,
                    parent_area.getY(),
                    w + 2 * padding, h + 2 * padding
                };
            }
        }

        void drawTooltip(juce::Graphics& g, const juce::String& text, const int width, const int height) override {
            juce::Rectangle<float> bound{static_cast<float>(width), static_cast<float>(height)};

            g.setColour(base_.getBackgroundColor().withAlpha(.875f));
            g.fillRect(bound);

            const auto padding = std::round(base_.getFontSize() * kPaddingScale);
            bound = bound.reduced(padding);
            const auto tl = getTipTextLayout(text, bound.getWidth(), bound.getHeight());
            tl.draw(g, bound);
        }

    private:
        UIBase& base_;

        static constexpr float kPaddingScale = .5f;

        [[nodiscard]] juce::TextLayout getTipTextLayout(const juce::String& text,
                                                        const float w, const float h) const {
            juce::AttributedString s;
            s.setJustification(juce::Justification::centredLeft);
            s.append(text, juce::FontOptions(base_.getFontSize() * 1.5f), base_.getTextColor());
            juce::TextLayout tl;
            tl.createLayout(s, w, h);
            return tl;
        }
    };
}
