// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLGUI_BUTTON_CUS_ATTACHMENT_HPP
#define ZLGUI_BUTTON_CUS_ATTACHMENT_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    /**
     * customized button attachment
     * @tparam sendClickNotification whether send click notification to button when para changes
     */
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

#endif //ZLGUI_BUTTON_CUS_ATTACHMENT_HPP
