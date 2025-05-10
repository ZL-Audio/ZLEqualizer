// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlgui {
    /**
     * customized button attachment
     * @tparam SendClickNotification whether send click notification to button when para changes
     */
    template<bool SendClickNotification>
    class ButtonCusAttachment final : private juce::Button::Listener {
    public:
        ButtonCusAttachment(const juce::AudioProcessorValueTreeState &state_to_use,
                            const juce::String &parameter_id,
                            juce::Button &b)
            : button_(b),
              stored_parameter_(*(state_to_use.getParameter(parameter_id))),
              attachment_(*(state_to_use.getParameter(parameter_id)),
                         [this](const float f) { setValue(f); },
                         state_to_use.undoManager) {
            sendInitialUpdate();
            button_.addListener(this);
        }

        ~ButtonCusAttachment() override {
            button_.removeListener(this);
        }

        void sendInitialUpdate() {
            attachment_.sendInitialUpdate();
        }

    private:
        void setValue(const float new_value) {
            const juce::ScopedValueSetter<bool> svs(ignore_callbacks_, true);
            if (SendClickNotification) {
                button_.setToggleState(new_value >= 0.5f, juce::sendNotificationSync);
            } else {
                button_.setToggleState(new_value >= 0.5f, juce::dontSendNotification);
            }
        }

        void buttonClicked(juce::Button *) override {
            if (ignore_callbacks_)
                return;

            attachment_.setValueAsCompleteGesture(button_.getToggleState() ? 1.0f : 0.0f);
        }

        juce::Button &button_;
        juce::RangedAudioParameter &stored_parameter_;
        juce::ParameterAttachment attachment_;
        bool ignore_callbacks_ = false;
    };
}
