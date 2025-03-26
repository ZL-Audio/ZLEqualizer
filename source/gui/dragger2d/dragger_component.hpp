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
#include "dragger_look_and_feel.hpp"

namespace zlInterface {
    class Dragger final : public juce::Component {
    public:
        std::function<juce::Point<float>(juce::Point<float> currentC, juce::Point<float> nextC)> checkCenter;

        explicit Dragger(UIBase &base);

        ~Dragger() override;

        bool updateButton();

        bool updateButton(const juce::Point<float> &center);

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void setButtonArea(juce::Rectangle<float> bound);

        juce::Rectangle<float> getButtonArea() const { return buttonArea; }

        juce::ToggleButton &getButton() { return button; }

        juce::Point<float> getButtonPos() const { return juce::Point<float>(previousX, previousY); }

        void setXPortion(float x);

        void setYPortion(float y);

        float getXPortion() const;

        float getYPortion() const;

        void setScale(const float x) { scale = x; }

        void visibilityChanged() override;

        class Listener {
        public:
            virtual ~Listener() = default;

            virtual void draggerValueChanged(Dragger *dragger) = 0;

            virtual void dragStarted(Dragger *dragger) = 0;

            virtual void dragEnded(Dragger *dragger) = 0;
        };

        /** Adds a listener to be called when this slider's value changes. */
        void addListener(Listener *listener);

        /** Removes a previously-registered listener. */
        void removeListener(Listener *listener);

        DraggerLookAndFeel &getLAF() { return draggerLAF; }

        void setXYEnabled(const bool x, const bool y) {
            if (x && !y) {
                checkCenter = [](const juce::Point<float> currentC,
                                 const juce::Point<float> nextC) {
                    return juce::Point<float>(nextC.x, currentC.y);
                };
            } else if (!x && y) {
                checkCenter = [](const juce::Point<float> currentC,
                                 const juce::Point<float> nextC) {
                    return juce::Point<float>(currentC.x, nextC.y);
                };
            }
        }

    private:
        UIBase &uiBase;
        bool isShiftDown{false};
        DraggerLookAndFeel draggerLAF;
        juce::ToggleButton button;
        float xPortion{0.f}, yPortion{0.f};
        float scale{1.f};

        juce::Point<float> mouseDownPos{}, currentPos{};
        juce::Rectangle<float> buttonArea{};

        float previousX{-100000.f}, previousY{-100000.f};

        juce::ListenerList<Listener> listeners;
    };
} // zlInterface
