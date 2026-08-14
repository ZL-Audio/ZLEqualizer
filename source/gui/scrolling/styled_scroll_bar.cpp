// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "styled_scroll_bar.hpp"

namespace zlgui::scrolling {
    class StyledScrollBar::LookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit LookAndFeel(UIBase& base) : base_(base) {
        }

        int getDefaultScrollbarWidth() override {
            return StyledScrollBar::getThickness(base_.getFontSize());
        }

        int getMinimumScrollbarThumbSize(juce::ScrollBar&) override {
            return juce::roundToInt(base_.getFontSize() * 2.f);
        }

        bool areScrollbarButtonsVisible() override {
            return false;
        }

        void drawScrollbar(juce::Graphics& g, juce::ScrollBar&, const int x, const int y,
                           const int width, const int height, const bool is_scrollbar_vertical,
                           const int thumb_start_position, const int thumb_size,
                           const bool is_mouse_over, const bool is_mouse_down) override {
            const auto font_size = base_.getFontSize();
            const auto area = juce::Rectangle<int>{x, y, width, height};
            const auto track_width = juce::jmax(1.f, font_size * .25f);
            const auto thumb_width = juce::jmax(2, juce::roundToInt(font_size * .25f));

            g.setColour(base_.getTextColour().withAlpha(.055f));
            if (is_scrollbar_vertical) {
                const auto track = area.toFloat().withSizeKeepingCentre(track_width,
                                                                        static_cast<float>(height));
                g.fillRoundedRectangle(track, track_width * .5f);
            } else {
                const auto track = area.toFloat().withSizeKeepingCentre(static_cast<float>(width),
                                                                        track_width);
                g.fillRoundedRectangle(track, track_width * .5f);
            }

            if (thumb_size <= 0) {
                return;
            }

            g.setColour(base_.getTextColour().withAlpha(is_mouse_down ? .42f
                                                       : is_mouse_over ? .3f : .2f));
            if (is_scrollbar_vertical) {
                const auto visual_width = juce::jmin(width, thumb_width);
                const auto thumb = juce::Rectangle<int>{area.getCentreX() - visual_width / 2,
                                                        thumb_start_position,
                                                        visual_width, thumb_size};
                g.fillRoundedRectangle(thumb.toFloat(), static_cast<float>(visual_width) * .5f);
            } else {
                const auto visual_height = juce::jmin(height, thumb_width);
                const auto thumb = juce::Rectangle<int>{thumb_start_position,
                                                        area.getCentreY() - visual_height / 2,
                                                        thumb_size, visual_height};
                g.fillRoundedRectangle(thumb.toFloat(), static_cast<float>(visual_height) * .5f);
            }
        }

    private:
        UIBase& base_;
    };

    StyledScrollBar::StyledScrollBar(UIBase& base, const bool is_vertical) :
        juce::ScrollBar(is_vertical), look_and_feel_(std::make_unique<LookAndFeel>(base)) {
        setAutoHide(true);
        setLookAndFeel(look_and_feel_.get());
    }

    StyledScrollBar::~StyledScrollBar() {
        setLookAndFeel(nullptr);
    }

    int StyledScrollBar::getThickness(const float font_size) {
        return juce::roundToInt(font_size * .5f);
    }

    int StyledScrollBar::getContentGap(const float font_size) {
        return juce::roundToInt(font_size * .16f);
    }
}
