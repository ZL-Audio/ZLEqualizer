// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlgui {
    class LeftRightButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit LeftRightButtonLookAndFeel(UIBase &base) : ui_base_(base) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool should_draw_button_as_highlighted, bool should_draw_button_as_down) override {
            juce::ignoreUnused(button, should_draw_button_as_highlighted, should_draw_button_as_down);

            if (should_draw_button_as_down && editable_) {
                g.setColour(ui_base_.getTextColor());
            } else {
                g.setColour(ui_base_.getTextInactiveColor());
            }

            juce::Path path;
            const auto d = direction_;
            const auto bound = button.getLocalBounds().toFloat();
            if (d < 0.1f) {
                path.startNewSubPath(bound.getTopLeft());
                path.lineTo(bound.getBottomLeft());
                path.lineTo(bound.getRight(), bound.getCentreY());
            } else if (0.2f < d && d < 0.3f) {
            } else if (0.45f < d && d < 0.55f) {
                path.startNewSubPath(bound.getTopRight());
                path.lineTo(bound.getBottomRight());
                path.lineTo(bound.getX(), bound.getCentreY());
            } else if (0.7f < d && d < 0.8f) {
            }
            path.closeSubPath();
            g.fillPath(path);
        }

        inline void setEditable(const bool f) { editable_ = f; }

        inline float getDepth() const { return button_depth_; }

        inline void setDepth(const float x) { button_depth_ = x; }

        inline void setDirection(const float x) { direction_ = x; }

    private:
        bool editable_{true};
        float button_depth_{0.f}, direction_{0.f};

        UIBase &ui_base_;
    };
}
