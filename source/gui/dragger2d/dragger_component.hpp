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

namespace zlgui {
    class Dragger final : public juce::Component {
    public:
        std::function<juce::Point<float>(juce::Point<float> currentC, juce::Point<float> nextC)> check_center_;

        explicit Dragger(UIBase &base);

        ~Dragger() override;

        bool updateButton();

        bool updateButton(const juce::Point<float> &center);

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void setButtonArea(juce::Rectangle<float> bound);

        juce::Rectangle<float> getButtonArea() const { return button_area_; }

        juce::ToggleButton &getButton() { return button_; }

        juce::Point<float> getButtonPos() const { return button_pos_; }

        void setXPortion(float x);

        void setYPortion(float y);

        float getXPortion() const;

        float getYPortion() const;

        void setScale(const float x) { scale_ = x; }

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

        DraggerLookAndFeel &getLAF() { return dragger_laf_; }

        void setXYEnabled(const bool x, const bool y) {
            if (x && !y) {
                check_center_ = [](const juce::Point<float> currentC,
                                 const juce::Point<float> nextC) {
                    return juce::Point<float>(nextC.x, currentC.y);
                };
            } else if (!x && y) {
                check_center_ = [](const juce::Point<float> currentC,
                                 const juce::Point<float> nextC) {
                    return juce::Point<float>(currentC.x, nextC.y);
                };
            }
        }

    private:
        UIBase &ui_base_;
        DraggerLookAndFeel dragger_laf_;
        juce::ToggleButton button_;
        float x_portion_{0.f}, y_portion_{0.f};
        float scale_{1.f};

        juce::Point<float> global_pos_{}, current_pos_{};
        juce::Rectangle<float> button_area_{};
        juce::Point<float> button_pos_{-100000.f, -100000.f};

        juce::ListenerList<Listener> listeners_;
    };
} // zlgui
