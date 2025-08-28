// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "component_updater.hpp"

namespace zlgui::attachment {
    template<bool UpdateFromAPVTS = true>
    class ButtonAttachment final : public ComponentAttachment,
                                   private juce::AudioProcessorValueTreeState::Listener,
                                   private juce::Button::Listener {
    public:
        ButtonAttachment(juce::Button &button,
                         juce::AudioProcessorValueTreeState &apvts,
                         const juce::String &parameter_ID,
                         ComponentUpdater &updater,
                         const juce::NotificationType notification_type = juce::NotificationType::sendNotificationSync)
            : button_(button), notification_type_(notification_type),
              apvts_(apvts), parameter_ref_(*apvts_.getParameter(parameter_ID)),
              updater_ref_(updater) {
            // add button listener
            button_.addListener(this);
            // add parameter listener
            if constexpr (UpdateFromAPVTS) {
                apvts_.addParameterListener(parameter_ref_.getParameterID(), this);
            }
            parameterChanged(parameter_ref_.getParameterID(),
                                 apvts_.getRawParameterValue(
                                     parameter_ref_.getParameterID())->load(std::memory_order::relaxed));
            if constexpr (UpdateFromAPVTS) {
                updater_ref_.addAttachment(*this);
            } else {
                updateComponent();
            }
        }

        ~ButtonAttachment() override {
            if constexpr (UpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ref_.getParameterID(), this);
            }
            button_.removeListener(this);
        }

        void updateComponent() override {
            const auto current_flag = atomic_flag_.load(std::memory_order::relaxed);
            if (current_flag != button_.getToggleState()) {
                button_.setToggleState(current_flag, notification_type_);
            }
        }

    private:
        juce::Button &button_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState &apvts_;
        juce::RangedAudioParameter &parameter_ref_;
        ComponentUpdater &updater_ref_;
        std::atomic<bool> atomic_flag_{false};

        void parameterChanged(const juce::String &, const float new_value) override {
            atomic_flag_.store(new_value > .5f, std::memory_order::relaxed);
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void buttonStateChanged(juce::Button *) override {
            parameter_ref_.beginChangeGesture();
            parameter_ref_.setValueNotifyingHost(static_cast<float>(button_.getToggleState()));
            parameter_ref_.endChangeGesture();
        }

        void buttonClicked(juce::Button *) override {
        }
    };
}
