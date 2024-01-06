// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_LEFT_RIGHT_BUTTON_LOOK_AND_FEEL_HPP
#define ZLEqualizer_LEFT_RIGHT_BUTTON_LOOK_AND_FEEL_HPP

namespace zlInterface {
    class LeftRightButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit LeftRightButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            if (shouldDrawButtonAsDown && editable.load()) {
                g.setColour(uiBase.getTextColor());
            } else {
                g.setColour(uiBase.getTextInactiveColor());
            }

            juce::Path path;
            const auto d = direction.load();
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

        inline void setEditable(const bool f) { editable.store(f); }

        inline float getDepth() const { return buttonDepth.load(); }

        inline void setDepth(const float x) { buttonDepth = x; }

        inline void setDirection(const float x) { direction.store(x); }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> buttonDepth = 0.f, direction=0.f;

        UIBase &uiBase;
    };
}

#endif //ZLEqualizer_LEFT_RIGHT_BUTTON_LOOK_AND_FEEL_HPP
