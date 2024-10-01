// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "reset_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    ResetAttach<FloatType>::ResetAttach(juce::AudioProcessor &processor,
                                        juce::AudioProcessorValueTreeState &parameters,
                                        juce::AudioProcessorValueTreeState &parametersNA,
                                        Controller<FloatType> &controller)
        : processorRef(processor),
          parameterRef(parameters), parameterNARef(parametersNA),
          controllerRef(controller) {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            parameterRef.addParameterListener(zlDSP::appendSuffix(zlDSP::bypass::ID, i), this);
            parameterNARef.addParameterListener(zlState::appendSuffix(zlState::active::ID, i), this);
        }
    }

    template<typename FloatType>
    ResetAttach<FloatType>::~ResetAttach() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            parameterRef.removeParameterListener(zlDSP::appendSuffix(zlDSP::bypass::ID, i), this);
            parameterNARef.removeParameterListener(zlState::appendSuffix(zlState::active::ID, i), this);
        }
    }

    template<typename FloatType>
    void ResetAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(zlDSP::bypass::ID)) {
            if (newValue < .5f) {
                auto *para = parameterNARef.getParameter(zlState::appendSuffix(zlState::active::ID, idx));
                para->beginChangeGesture();
                para->setValueNotifyingHost(zlState::active::convertTo01(true));
                para->endChangeGesture();
            }
        } else if (parameterID.startsWith(zlState::active::ID)) {
            const auto active = newValue > .5f;
            if (newValue < .5f) {
                const auto suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
                for (size_t j = 0; j < resetDefaultVs.size(); ++j) {
                    const auto resetID = resetIDs[j] + suffix;
                    parameterRef.getParameter(resetID)->beginChangeGesture();
                    parameterRef.getParameter(resetID)->setValueNotifyingHost(resetDefaultVs[j]);
                    parameterRef.getParameter(resetID)->endChangeGesture();
                }
            }
            controllerRef.setIsActive(idx, active);
        }
    }

    template
    class ResetAttach<float>;

    template
    class ResetAttach<double>;
} // zlDSP
