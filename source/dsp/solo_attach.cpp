// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    SoloAttach<FloatType>::SoloAttach(juce::AudioProcessor &processor,
                                      juce::AudioProcessorValueTreeState &parameters,
                                      Controller<FloatType> &controller) : processorRef(processor),
                                                                           parameterRef(parameters),
                                                                           controllerRef(controller) {
        addListeners();
    }

    template<typename FloatType>
    SoloAttach<FloatType>::~SoloAttach() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (auto &ID: IDs) {
                parameterRef.removeParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::addListeners() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (auto &ID: IDs) {
                parameterRef.addParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto id = parameterID.dropLastCharacters(2);
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        const auto value = static_cast<FloatType>(newValue);
        if (id == solo::ID || id == sideSolo::ID) {
            const auto isSide = (id == sideSolo::ID);
            if (static_cast<bool>(value)) {
                if (controllerRef.getSolo() && (idx != controllerRef.getSoloIdx() ||
                                                isSide != controllerRef.getSoloIsSide())) {
                    auto initID = controllerRef.getSoloIsSide()
                                      ? sideSolo::ID + parameterID.getLastCharacters(2)
                                      : solo::ID + parameterID.getLastCharacters(2);
                    parameterRef.getParameter(initID)->beginChangeGesture();
                    parameterRef.getParameter(initID)->setValueNotifyingHost(static_cast<float>(false));
                    parameterRef.getParameter(initID)->endChangeGesture();
                }
                controllerRef.setSolo(idx, isSide);
            } else {
                controllerRef.clearSolo();
            }
        } else {
            if (controllerRef.getSolo()) {
                triggerAsyncUpdate();
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::handleAsyncUpdate() {
        controllerRef.setSolo(controllerRef.getSoloIdx(), controllerRef.getSoloIsSide());
    }

    template
    class SoloAttach<float>;

    template
    class SoloAttach<double>;
}
