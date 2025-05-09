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
                             juce::AudioProcessorValueTreeState &parametersNA)
            : idx(bandIdx), parameters_ref(parameters), parameters_NA_ref(parametersNA) {
            parameters_ref.addParameterListener(zlp::appendSuffix(zlp::bypass::ID, idx), this);
            parameters_NA_ref.addParameterListener(zlstate::appendSuffix(zlstate::active::ID, idx), this);
        }

        ~ResetAttach() override {
            parameters_ref.removeParameterListener(zlp::appendSuffix(zlp::bypass::ID, idx), this);
            parameters_NA_ref.removeParameterListener(zlstate::appendSuffix(zlstate::active::ID, idx), this);
        }

    private:
        size_t idx;
        juce::AudioProcessorValueTreeState &parameters_ref;
        juce::AudioProcessorValueTreeState &parameters_NA_ref;
        std::atomic<bool> toActive;

        constexpr static std::array resetIDs{
            zlp::solo::ID, zlp::sideSolo::ID,
            zlp::dynamicON::ID, zlp::dynamicLearn::ID,
            zlp::threshold::ID, zlp::kneeW::ID, zlp::attack::ID, zlp::release::ID,
            zlp::bypass::ID, zlp::fType::ID, zlp::slope::ID, zlp::lrType::ID
        };

        inline const static std::array resetDefaultVs{
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

        void parameterChanged(const juce::String &parameterID, float newValue) override {
            if (parameterID.startsWith(zlp::bypass::ID) && newValue < .5f) {
                toActive.store(true);
                triggerAsyncUpdate();
            } else if (parameterID.startsWith(zlstate::active::ID) && newValue < .5f) {
                toActive.store(false);
                triggerAsyncUpdate();
            }
        }

        void handleAsyncUpdate() override {
            if (toActive.load()) {
                auto *para = parameters_NA_ref.getParameter(zlstate::appendSuffix(zlstate::active::ID, idx));
                para->beginChangeGesture();
                para->setValueNotifyingHost(zlstate::active::convertTo01(true));
                para->endChangeGesture();
            } else {
                const auto suffix = zlstate::appendSuffix("", idx);
                for (size_t j = 0; j < resetDefaultVs.size(); ++j) {
                    const auto resetID = resetIDs[j] + suffix;
                    auto *para = parameters_ref.getParameter(resetID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(resetDefaultVs[j]);
                    para->endChangeGesture();
                }
            }
        }
    };
}
