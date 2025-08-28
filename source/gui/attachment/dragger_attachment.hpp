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
#include "../dragger/dragger.hpp"

namespace zlgui::attachment {
    template<bool UpdateFromAPVTS = true>
    class DraggerAttachment final : public ComponentAttachment,
                                    private juce::AudioProcessorValueTreeState::Listener,
                                    private dragger::Dragger::Listener {
    public:
        DraggerAttachment(dragger::Dragger &dragger, juce::AudioProcessorValueTreeState &apvts,
                          const juce::String &parameter_x_ID, juce::NormalisableRange<float> normalisable_range_x,
                          const juce::String &parameter_y_ID, juce::NormalisableRange<float> normalisable_range_y,
                          ComponentUpdater &updater)
            : dragger_(dragger), apvts_(apvts),
              parameter_x_ref_(*apvts.getParameter(parameter_x_ID)),
              parameter_y_ref_(*apvts.getParameter(parameter_y_ID)),
              normalisable_range_x_(normalisable_range_x),
              normalisable_range_y_(normalisable_range_y),
              updater_ref_(updater) {
            // add parameter listener
            if constexpr (UpdateFromAPVTS) {
                apvts.addParameterListener(parameter_x_ref_.getParameterID(), this);
                apvts.addParameterListener(parameter_y_ref_.getParameterID(), this);
            }
            parameterChanged(parameter_x_ref_.getParameterID(),
                             apvts.getRawParameterValue(parameter_x_ref_.getParameterID())->load(
                                 std::memory_order::relaxed));
            parameterChanged(parameter_y_ref_.getParameterID(),
                             apvts.getRawParameterValue(parameter_y_ref_.getParameterID())->load(
                                 std::memory_order::relaxed));
            if constexpr (UpdateFromAPVTS) {
                updater_ref_.addAttachment(*this);
            } else {
                updateComponent();
            }
            // add dragger listener
            dragger_.addListener(this);
        }

        ~DraggerAttachment() override {
            if constexpr (UpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_x_ref_.getParameterID(), this);
                apvts_.removeParameterListener(parameter_y_ref_.getParameterID(), this);
            }
            dragger_.removeListener(this);
        }

        void updateComponent() override {
            const auto new_x = normalisable_range_x_.convertTo0to1(atomic_x_.load(std::memory_order::relaxed));
            const auto new_y = normalisable_range_y_.convertTo0to1(atomic_y_.load(std::memory_order::relaxed));
            if (std::abs(dragger_.getXPortion() - new_x) > 0.001f || std::abs(dragger_.getYPortion() - new_y) >
                0.001f) {
                dragger_.setXPortion(new_x);
                dragger_.setYPortion(new_y);
                dragger_.updateButton();
            }
        }

    private:
        dragger::Dragger &dragger_;
        juce::AudioProcessorValueTreeState &apvts_;
        juce::RangedAudioParameter &parameter_x_ref_, &parameter_y_ref_;
        juce::NormalisableRange<float> normalisable_range_x_, normalisable_range_y_;
        ComponentUpdater &updater_ref_;
        std::atomic<float> atomic_x_{0.f}, atomic_y_{0.f};

        void parameterChanged(const juce::String &parameter_ID, float new_value) override {
            if (parameter_ID == parameter_x_ref_.getParameterID()) {
                atomic_x_.store(new_value, std::memory_order::relaxed);
            } else if (parameter_ID == parameter_y_ref_.getParameterID()) {
                atomic_y_.store(new_value, std::memory_order::relaxed);
            }
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void dragStarted(dragger::Dragger *) override {
            parameter_x_ref_.beginChangeGesture();
            parameter_y_ref_.beginChangeGesture();
        }

        void dragEnded(dragger::Dragger *) override {
            parameter_x_ref_.endChangeGesture();
            parameter_y_ref_.endChangeGesture();
        }

        void draggerValueChanged(dragger::Dragger *) override {
            parameter_x_ref_.setValueNotifyingHost(
                parameter_x_ref_.convertTo0to1(
                    normalisable_range_x_.convertFrom0to1(
                        std::clamp(dragger_.getXPortion(), 0.f, 1.f))));
            parameter_y_ref_.setValueNotifyingHost(
                parameter_y_ref_.convertTo0to1(
                    normalisable_range_y_.convertFrom0to1(
                        std::clamp(dragger_.getYPortion(), 0.f, 1.f))));
        }
    };
}
