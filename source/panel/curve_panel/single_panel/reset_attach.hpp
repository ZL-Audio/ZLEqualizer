// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"

namespace zlpanel {
    class ResetAttach final : private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit ResetAttach(const size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA)
            : band_idx_(bandIdx), parameters_ref_(parameters), parameters_NA_ref_(parameters_NA) {
            parameters_ref_.addParameterListener(zlp::appendSuffix(zlp::bypass::ID, band_idx_), this);
            parameters_NA_ref_.addParameterListener(zlstate::appendSuffix(zlstate::active::ID, band_idx_), this);
        }

        ~ResetAttach() override {
            parameters_ref_.removeParameterListener(zlp::appendSuffix(zlp::bypass::ID, band_idx_), this);
            parameters_NA_ref_.removeParameterListener(zlstate::appendSuffix(zlstate::active::ID, band_idx_), this);
        }

    private:
        size_t band_idx_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        std::atomic<bool> to_active_;

        static constexpr std::array kResetIDs{
            zlp::solo::ID, zlp::sideSolo::ID,
            zlp::dynamicON::ID, zlp::dynamicLearn::ID,
            zlp::threshold::ID, zlp::kneeW::ID, zlp::attack::ID, zlp::release::ID,
            zlp::bypass::ID, zlp::fType::ID, zlp::slope::ID, zlp::lrType::ID
        };

        inline const static std::array kResetDefaultVs{
            zlp::solo::convertTo01(zlp::solo::defaultV),
            zlp::sideSolo::convertTo01(zlp::sideSolo::defaultV),
            zlp::dynamicON::convertTo01(zlp::dynamicON::defaultV),
            zlp::dynamicLearn::convertTo01(zlp::dynamicLearn::defaultV),
            zlp::threshold::convertTo01(zlp::threshold::defaultV),
            zlp::kneeW::convertTo01(zlp::kneeW::defaultV),
            zlp::attack::convertTo01(zlp::attack::defaultV),
            zlp::release::convertTo01(zlp::release::defaultV),
            zlp::bypass::convertTo01(zlp::bypass::defaultV),
            zlp::fType::convertTo01(zlp::fType::defaultI),
            zlp::slope::convertTo01(zlp::slope::defaultI),
            zlp::lrType::convertTo01(zlp::lrType::defaultI),
        };

        void parameterChanged(const juce::String &parameter_id, float new_value) override {
            if (parameter_id.startsWith(zlp::bypass::ID) && new_value < .5f) {
                to_active_.store(true);
                triggerAsyncUpdate();
            } else if (parameter_id.startsWith(zlstate::active::ID) && new_value < .5f) {
                to_active_.store(false);
                triggerAsyncUpdate();
            }
        }

        void handleAsyncUpdate() override {
            if (to_active_.load()) {
                auto *para = parameters_NA_ref_.getParameter(zlstate::appendSuffix(zlstate::active::ID, band_idx_));
                para->beginChangeGesture();
                para->setValueNotifyingHost(zlstate::active::convertTo01(true));
                para->endChangeGesture();
            } else {
                const auto suffix = zlstate::appendSuffix("", band_idx_);
                for (size_t j = 0; j < kResetDefaultVs.size(); ++j) {
                    auto *para = parameters_ref_.getParameter(kResetIDs[j] + suffix);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(kResetDefaultVs[j]);
                    para->endChangeGesture();
                }
            }
        }
    };
}
