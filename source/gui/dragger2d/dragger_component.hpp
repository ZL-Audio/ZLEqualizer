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
#include "dragger_constrainer.hpp"

namespace zlInterface {
    class Dragger final : public juce::Component {
    public:
        explicit Dragger(UIBase &base);

        ~Dragger() override;

        void paint(juce::Graphics &g) override;

        bool updateButton();

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void resized() override;

        juce::ToggleButton &getButton() { return button; }

        void setXPortion(float x);

        void setYPortion(float y);

        float getXPortion() const;

        float getYPortion() const;

        void setScale(const float x) { scale.store(x); }

        float getScale() const { return scale.load(); }

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

        void setPadding(const float left, const float right,
                        const float upper, const float bottom) {
            lPadding = left;
            rPadding = right;
            uPadding = upper;
            bPadding = bottom;
        }

        void setActive(const bool f) { draggerLAF.setActive(f); }

        DraggerLookAndFeel &getLAF() { return draggerLAF; }

        void setXYEnabled(const bool x, const bool y) {
            xEnabled = x;
            yEnabled = y;
        }

    private:
        UIBase &uiBase;

        juce::Component preButton, dummyButton;
        juce::Rectangle<int> preBound, dummyBound;
        bool isShiftDown{false};
        juce::ToggleButton button;
        std::atomic<bool> dummyButtonChanged{false};
        DraggerLookAndFeel draggerLAF;
        juce::ComponentDragger dragger;
        DraggerConstrainer constrainer;
        bool xEnabled{true}, yEnabled{true};
        std::atomic<bool> isSelected;
        std::atomic<float> xPortion, yPortion;

        juce::Rectangle<float> buttonArea;
        float lPadding{0.f}, rPadding{0.f}, uPadding{0.f}, bPadding{0.f};
        std::atomic<float> scale{1.f};

        juce::ListenerList<Listener> listeners;
    };
} // zlInterface
