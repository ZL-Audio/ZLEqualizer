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
    class DraggerLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        enum DraggerShape {
            round,
            rectangle,
            upDownArrow,
            rightArrow,
            leftArrow
        };

        explicit DraggerLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            if (!active.load()) { return; }

            if (shouldDrawButtonAsDown || button.getToggleState()) {
                g.setColour(uiBase.getTextColor());
                g.fillPath(outlinePath);
            } else if (shouldDrawButtonAsHighlighted || isSelected.load()) {
                g.setColour(uiBase.getTextColor().withMultipliedAlpha(0.5f));
                g.fillPath(outlinePath);
            }

            g.setColour(colour);
            g.fillPath(innerPath);

            if (label.load() != ' ') {
                const auto l = std::string{label.load()};
                if (colour.getPerceivedBrightness() <= .5f) {
                    g.setColour(juce::Colours::white);
                } else {
                    g.setColour(juce::Colours::black);
                }
                g.setFont(uiBase.getFontSize() * labelScale);
                auto bound = button.getLocalBounds().toFloat();
                const auto radius = std::min(bound.getHeight(), bound.getWidth());
                bound = bound.withSizeKeepingCentre(radius, radius);
                g.drawText(l, bound, juce::Justification::centred);
            }
        }

        inline void setColour(const juce::Colour c) { colour = c; }

        void setActive(const bool f) { active.store(f); }

        bool getActive() const { return active.load(); }

        void setIsSelected(const bool f) { isSelected.store(f); }

        bool getIsSelected() const { return isSelected.load(); }

        void setDraggerShape(const DraggerShape s) { draggerShape.store(s); }

        void updatePaths(juce::Rectangle<float> &bound) {
            outlinePath.clear();
            innerPath.clear();
            switch (draggerShape.load()) {
                case round: {
                    updateRoundPaths(bound);
                    break;
                }
                case rectangle: {
                    updateRectanglePaths(bound);
                    break;
                }
                case upDownArrow: {
                    updateUpDownArrowPaths(bound);
                    break;
                }
                case rightArrow: {
                    updateRightArrowPaths(bound);
                    break;
                }
                case leftArrow: {
                    updateLeftArrowPaths(bound);
                    break;
                }
            }
        }

        void updateRoundPaths(juce::Rectangle<float> &bound) {
            const auto radius = bound.getWidth();
            outlinePath.addEllipse(bound);
            bound = bound.withSizeKeepingCentre(radius * .75f, radius * .75f);
            innerPath.addEllipse(bound);
        }

        void updateRectanglePaths(juce::Rectangle<float> &bound) {
            const auto radius = bound.getWidth() * 0.75f;
            bound = bound.withSizeKeepingCentre(radius, radius);
            outlinePath.addRectangle(bound);
            bound = bound.withSizeKeepingCentre(radius * .7f, radius * .7f);
            innerPath.addRectangle(bound);
        }

        void updateUpDownArrowPaths(juce::Rectangle<float> &bound) {
            auto updateOnePath = [](juce::Path &path, const juce::Rectangle<float> &temp) {
                path.startNewSubPath(temp.getCentreX(), temp.getY());
                path.lineTo(temp.getCentreX() + temp.getWidth() * .33f, temp.getCentreY());
                path.lineTo(temp.getCentreX(), temp.getBottom());
                path.lineTo(temp.getCentreX() - temp.getWidth() * .33f, temp.getCentreY());
                path.closeSubPath();
            };
            updateOnePath(outlinePath, bound);
            bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * .75f);
            updateOnePath(innerPath, bound);
        }

        void updateRightArrowPaths(juce::Rectangle<float> &bound) {
            auto updateOnePath = [](juce::Path &path, const juce::Rectangle<float> &temp) {
                const auto center = temp.getCentre();
                path.startNewSubPath(center.getX() + temp.getWidth() * .5f, center.getY());
                path.lineTo(center.getX(), center.getY() + temp.getHeight() * std::sqrt(3.f) * .25f);
                path.lineTo(center.getX(), center.getY() - temp.getHeight() * std::sqrt(3.f) * .25f);
                path.closeSubPath();
            };
            updateOnePath(outlinePath, bound);
            bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * .75f);
            updateOnePath(innerPath, bound);
        }

        void updateLeftArrowPaths(juce::Rectangle<float> &bound) {
            auto updateOnePath = [](juce::Path &path, const juce::Rectangle<float> &temp) {
                const auto center = temp.getCentre();
                path.startNewSubPath(center.getX() - temp.getWidth() * .5f, center.getY());
                path.lineTo(center.getX(), center.getY() + temp.getHeight() * std::sqrt(3.f) * .25f);
                path.lineTo(center.getX(), center.getY() - temp.getHeight() * std::sqrt(3.f) * .25f);
                path.closeSubPath();
            };
            updateOnePath(outlinePath, bound);
            bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * .75f);
            updateOnePath(innerPath, bound);
        }

        void setLabel(const char l) { label.store(l); }

        void setLabelScale(const float x) { labelScale = x; }

    private:
        juce::Colour colour;
        juce::Path outlinePath, innerPath;
        std::atomic<bool> active{true}, isSelected{false};
        std::atomic<DraggerShape> draggerShape{DraggerShape::round};
        std::atomic<char> label;
        float labelScale = 1.f;
        UIBase &uiBase;
    };
}
