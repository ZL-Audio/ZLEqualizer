// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlInterface {
    class LeftRightButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit LeftRightButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            if (shouldDrawButtonAsDown && editable) {
                g.setColour(uiBase.getTextColor());
            } else {
                g.setColour(uiBase.getTextInactiveColor());
            }

            juce::Path path;
            const auto d = direction;
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

        inline void setEditable(const bool f) { editable = f; }

        inline float getDepth() const { return buttonDepth; }

        inline void setDepth(const float x) { buttonDepth = x; }

        inline void setDirection(const float x) { direction = x; }

    private:
        bool editable{true};
        float buttonDepth{0.f}, direction{0.f};

        UIBase &uiBase;
    };
}
