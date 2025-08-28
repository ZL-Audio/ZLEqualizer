// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <utility>

#include "component_updater.hpp"

namespace zlgui::attachment {
    template<bool UpdateFromAPVTS = true>
    class SliderAttachment final : public ComponentAttachment,
                                   private juce::AudioProcessorValueTreeState::Listener,
                                   private juce::Slider::Listener {
    public:
        SliderAttachment(juce::Slider &slider,
                         juce::AudioProcessorValueTreeState &apvts,
                         const juce::String &parameter_ID,
                         ComponentUpdater &updater,
                         const juce::NotificationType notification_type = juce::NotificationType::sendNotificationSync)
            : slider_(slider), notification_type_(notification_type),
              apvts_(apvts), parameter_ref_(*apvts_.getParameter(parameter_ID)),
              updater_ref_(updater) {
            // setup slider values
            const auto &para_ref(parameter_ref_);
            slider_.valueFromTextFunction = [&para_ref](const juce::String &text) {
                return static_cast<double>(para_ref.convertFrom0to1(para_ref.getValueForText(text)));
            };
            slider_.textFromValueFunction = [&para_ref](const double value) {
                return para_ref.getText(para_ref.convertTo0to1(static_cast<float>(value)), 0);
            };
            slider_.setDoubleClickReturnValue(slider_.isDoubleClickReturnEnabled(),
                                              parameter_ref_.convertFrom0to1(parameter_ref_.getDefaultValue()));

            // setup slider range
            auto range = parameter_ref_.getNormalisableRange();
            auto convert_from_0to1 = [range](const double, const double, const double normalised_value) mutable {
                return static_cast<double>(range.convertFrom0to1(static_cast<float>(normalised_value)));
            };
            auto convert_to_0to1 = [range](const double, const double, const double mapped_value) mutable {
                return static_cast<double>(range.convertTo0to1(static_cast<float>(mapped_value)));
            };
            auto snap_to_legal_value = [range](const double, const double, const double mapped_value) mutable {
                return static_cast<double>(range.snapToLegalValue(static_cast<float>(mapped_value)));
            };
            juce::NormalisableRange<double> new_range{
                static_cast<double>(range.start),
                static_cast<double>(range.end),
                std::move(convert_from_0to1),
                std::move(convert_to_0to1),
                std::move(snap_to_legal_value)
            };
            new_range.interval = range.interval;
            new_range.skew = range.skew;
            new_range.symmetricSkew = range.symmetricSkew;
            slider_.setNormalisableRange(new_range);
            // add slider listener
            slider_.addListener(this);
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

        SliderAttachment(juce::Slider &slider,
                         juce::AudioProcessorValueTreeState &apvts,
                         const juce::String &parameter_ID,
                         const juce::NormalisableRange<double> &range,
                         ComponentUpdater &updater,
                         const juce::NotificationType notification_type = juce::NotificationType::sendNotificationSync)
            : slider_(slider), notification_type_(notification_type),
              apvts_(apvts), parameter_ref_(*apvts_.getParameter(parameter_ID)),
              updater_ref_(updater) {
            // setup slider values
            const auto &para_ref(parameter_ref_);
            slider_.valueFromTextFunction = [&para_ref](const juce::String &text) {
                return static_cast<double>(para_ref.convertFrom0to1(para_ref.getValueForText(text)));
            };
            slider_.textFromValueFunction = [&para_ref](const double value) {
                return para_ref.getText(para_ref.convertTo0to1(static_cast<float>(value)), 0);
            };
            slider_.setDoubleClickReturnValue(slider_.isDoubleClickReturnEnabled(),
                                              parameter_ref_.convertFrom0to1(parameter_ref_.getDefaultValue()));

            // setup slider range
            slider_.setNormalisableRange(range);
            // add slider listener
            slider_.addListener(this);
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

        ~SliderAttachment() override {
            if constexpr (UpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ref_.getParameterID(), this);
            }
            slider_.removeListener(this);
        }

        void updateComponent() override {
            const auto current_value = atomic_value_.load(std::memory_order::relaxed);
            if (std::abs(current_value - slider_.getValue()) > 1e-6f) {
                slider_.setValue(current_value, notification_type_);
            }
        }

    private:
        juce::Slider &slider_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState &apvts_;
        juce::RangedAudioParameter &parameter_ref_;
        ComponentUpdater &updater_ref_;
        std::atomic<float> atomic_value_{0.f};

        void parameterChanged(const juce::String &, const float new_value) override {
            atomic_value_.store(new_value, std::memory_order::relaxed);
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void sliderValueChanged(juce::Slider *) override {
            const auto normalized_value = parameter_ref_.convertTo0to1(static_cast<float>(slider_.getValue()));
            if (std::abs(normalized_value - parameter_ref_.getValue()) > 1e-6) {
                parameter_ref_.setValueNotifyingHost(normalized_value);
            }
        }

        void sliderDragStarted(juce::Slider *) override {
            parameter_ref_.beginChangeGesture();
        }

        void sliderDragEnded(juce::Slider *) override {
            parameter_ref_.endChangeGesture();
        }
    };
}
