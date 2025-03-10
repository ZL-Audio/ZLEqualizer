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

namespace zlPanel {
    class ResetAttach final : private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit ResetAttach(const size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA)
            : idx(bandIdx), parametersRef(parameters), parametersNARef(parametersNA) {
            parametersRef.addParameterListener(zlDSP::appendSuffix(zlDSP::bypass::ID, idx), this);
            parametersNARef.addParameterListener(zlState::appendSuffix(zlState::active::ID, idx), this);
        }

        ~ResetAttach() override {
            parametersRef.removeParameterListener(zlDSP::appendSuffix(zlDSP::bypass::ID, idx), this);
            parametersNARef.removeParameterListener(zlState::appendSuffix(zlState::active::ID, idx), this);
        }

    private:
        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef;
        juce::AudioProcessorValueTreeState &parametersNARef;
        std::atomic<bool> toActive;

        constexpr static std::array resetIDs{
            zlDSP::solo::ID, zlDSP::sideSolo::ID,
            zlDSP::dynamicON::ID, zlDSP::dynamicLearn::ID,
            zlDSP::threshold::ID, zlDSP::kneeW::ID, zlDSP::attack::ID, zlDSP::release::ID,
            zlDSP::bypass::ID, zlDSP::fType::ID, zlDSP::slope::ID, zlDSP::lrType::ID
        };

        inline const static std::array resetDefaultVs{
            zlDSP::solo::convertTo01(zlDSP::solo::defaultV),
            zlDSP::sideSolo::convertTo01(zlDSP::sideSolo::defaultV),
            zlDSP::dynamicON::convertTo01(zlDSP::dynamicON::defaultV),
            zlDSP::dynamicLearn::convertTo01(zlDSP::dynamicLearn::defaultV),
            zlDSP::threshold::convertTo01(zlDSP::threshold::defaultV),
            zlDSP::kneeW::convertTo01(zlDSP::kneeW::defaultV),
            zlDSP::attack::convertTo01(zlDSP::attack::defaultV),
            zlDSP::release::convertTo01(zlDSP::release::defaultV),
            zlDSP::bypass::convertTo01(zlDSP::bypass::defaultV),
            zlDSP::fType::convertTo01(zlDSP::fType::defaultI),
            zlDSP::slope::convertTo01(zlDSP::slope::defaultI),
            zlDSP::lrType::convertTo01(zlDSP::lrType::defaultI),
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override {
            if (parameterID.startsWith(zlDSP::bypass::ID) && newValue < .5f) {
                toActive.store(true);
                triggerAsyncUpdate();
            } else if (parameterID.startsWith(zlState::active::ID) && newValue < .5f) {
                toActive.store(false);
                triggerAsyncUpdate();
            }
        }

        void handleAsyncUpdate() override {
            if (toActive.load()) {
                auto *para = parametersNARef.getParameter(zlState::appendSuffix(zlState::active::ID, idx));
                para->beginChangeGesture();
                para->setValueNotifyingHost(zlState::active::convertTo01(true));
                para->endChangeGesture();
            } else {
                const auto suffix = zlState::appendSuffix("", idx);
                for (size_t j = 0; j < resetDefaultVs.size(); ++j) {
                    const auto resetID = resetIDs[j] + suffix;
                    auto *para = parametersRef.getParameter(resetID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(resetDefaultVs[j]);
                    para->endChangeGesture();
                }
            }
        }
    };
}
