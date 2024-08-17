// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef BUTTON_ATTACHMENT_HPP
#define BUTTON_ATTACHMENT_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    template<bool sendClickNotification>
    class ButtonCusAttachment final : private juce::Button::Listener {
    public:
        ButtonCusAttachment(const juce::AudioProcessorValueTreeState &stateToUse,
                            const juce::String &parameterID,
                            juce::Button &b)
            : button(b),
              storedParameter(*(stateToUse.getParameter(parameterID))),
              attachment(*(stateToUse.getParameter(parameterID)),
                         [this](const float f) { setValue(f); },
                         stateToUse.undoManager) {
            sendInitialUpdate();
            button.addListener(this);
        }

        ~ButtonCusAttachment() override {
            button.removeListener(this);
        }

        /** Call this after setting up your combo box in the case where you need to do
            extra setup after constructing this attachment.
        */
        void sendInitialUpdate() {
            attachment.sendInitialUpdate();
        }

    private:
        void setValue(const float newValue) {
            const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
            if (sendClickNotification) {
                button.setToggleState(newValue >= 0.5f, juce::sendNotificationSync);
            } else {
                button.setToggleState(newValue >= 0.5f, juce::dontSendNotification);
            }
        }

        void buttonClicked(juce::Button *) override {
            if (ignoreCallbacks)
                return;

            attachment.setValueAsCompleteGesture(button.getToggleState() ? 1.0f : 0.0f);
        }

        juce::Button &button;
        juce::RangedAudioParameter &storedParameter;
        juce::ParameterAttachment attachment;
        bool ignoreCallbacks = false;
    };
}

#endif //BUTTON_ATTACHMENT_HPP
