// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../attachment/component_updater.hpp"
#include "dragger.hpp"

namespace zlgui::attachment {
    template <bool kUpdateFromAPVTS = true, bool kXAttach = true>
    class DraggerAttachment final : public ComponentAttachment,
                                    private juce::AudioProcessorValueTreeState::Listener,
                                    private dragger::Dragger::Listener {
    public:
        DraggerAttachment(dragger::Dragger& dragger, juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& parameter_ID, juce::NormalisableRange<float> normalisable_range,
                          ComponentUpdater& updater) :
            dragger_(dragger), apvts_(apvts),
            parameter_ref_(*apvts.getParameter(parameter_ID)),
            normalisable_range_(normalisable_range),
            updater_ref_(updater) {
            // add parameter listener
            if constexpr (kUpdateFromAPVTS) {
                apvts.addParameterListener(parameter_ref_.getParameterID(), this);
                parameterChanged(parameter_ref_.getParameterID(),
                             apvts.getRawParameterValue(parameter_ref_.getParameterID())->load(
                                 std::memory_order::relaxed));
                updater_ref_.addAttachment(*this);
            }
            // add dragger listener
            dragger_.addListener(this);
        }

        ~DraggerAttachment() override {
            if constexpr (kUpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ref_.getParameterID(), this);
            }
            dragger_.removeListener(this);
        }

        void updateComponent() override {
            if constexpr (kUpdateFromAPVTS) {
                const auto new_p = normalisable_range_.convertTo0to1(atomic_.load(std::memory_order::relaxed));
                if constexpr (kXAttach) {
                    if (std::abs(dragger_.getXPortion() - new_p) > 0.001f) {
                        dragger_.setXPortion(new_p);
                        dragger_.updateButton();
                    }
                } else {
                    if (std::abs(dragger_.getYPortion() - new_p) > 0.001f) {
                        dragger_.setYPortion(new_p);
                        dragger_.updateButton();
                    }
                }
            }
        }

    private:
        dragger::Dragger& dragger_;
        juce::AudioProcessorValueTreeState& apvts_;
        juce::RangedAudioParameter& parameter_ref_;
        juce::NormalisableRange<float> normalisable_range_;
        ComponentUpdater& updater_ref_;
        std::atomic<float> atomic_{0.f};

        void parameterChanged(const juce::String&, const float value) override {
            if constexpr (kUpdateFromAPVTS) {
                atomic_.store(value, std::memory_order::relaxed);
                updater_ref_.getFlag().store(true, std::memory_order::release);
            }
        }

        void dragStarted(dragger::Dragger*) override {
            parameter_ref_.beginChangeGesture();
        }

        void dragEnded(dragger::Dragger*) override {
            parameter_ref_.endChangeGesture();
        }

        void draggerValueChanged(dragger::Dragger*) override {
            if constexpr (kXAttach) {
                parameter_ref_.setValueNotifyingHost(
                    parameter_ref_.convertTo0to1(normalisable_range_.convertFrom0to1(
                        std::clamp(dragger_.getXPortion(), 0.f, 1.f))));
            } else {
                parameter_ref_.setValueNotifyingHost(
                    parameter_ref_.convertTo0to1(normalisable_range_.convertFrom0to1(
                        std::clamp(dragger_.getYPortion(), 0.f, 1.f))));
            }
        }
    };
}
