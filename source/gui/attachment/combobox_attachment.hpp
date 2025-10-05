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
    template <bool kUpdateFromAPVTS = true>
    class ComboBoxAttachment final : public ComponentAttachment,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ComboBox::Listener {
    public:
        ComboBoxAttachment(juce::ComboBox& box,
                           juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& parameter_ID,
                           ComponentUpdater& updater,
                           const juce::NotificationType notification_type =
                               juce::NotificationType::sendNotificationSync)
            : box_(box), notification_type_(notification_type),
              apvts_(apvts), parameter_ref_(*apvts_.getParameter(parameter_ID)),
              updater_ref_(updater) {
            // add combobox listener
            box_.addListener(this);
            // add parameter listener
            if constexpr (kUpdateFromAPVTS) {
                apvts_.addParameterListener(parameter_ref_.getParameterID(), this);
            }
            parameterChanged(parameter_ref_.getParameterID(),
                             apvts_.getRawParameterValue(
                                 parameter_ref_.getParameterID())->load(std::memory_order::relaxed));
            if constexpr (kUpdateFromAPVTS) {
                updater_ref_.addAttachment(*this);
            } else {
                updateComponent();
            }
        }

        ~ComboBoxAttachment() override {
            if constexpr (kUpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ref_.getParameterID(), this);
            }
            box_.removeListener(this);
        }

        void updateComponent() override {
            const auto current_index = atomic_index_.load(std::memory_order::relaxed);
            if (current_index != box_.getSelectedItemIndex()) {
                box_.setSelectedItemIndex(current_index, notification_type_);
            }
        }

    private:
        juce::ComboBox& box_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState& apvts_;
        juce::RangedAudioParameter& parameter_ref_;
        ComponentUpdater& updater_ref_;
        std::atomic<int> atomic_index_{0};

        void parameterChanged(const juce::String&, const float new_value) override {
            atomic_index_.store(static_cast<int>(new_value), std::memory_order::relaxed);
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void comboBoxChanged(juce::ComboBox*) override {
            parameter_ref_.beginChangeGesture();
            parameter_ref_.setValueNotifyingHost(
                parameter_ref_.convertTo0to1(static_cast<float>(box_.getSelectedItemIndex())));
            parameter_ref_.endChangeGesture();
        }
    };
}
