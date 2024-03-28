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
        initDefaultValues();
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
                    const auto oldIdx = controllerRef.getSoloIdx();
                    const auto oldSuffix = oldIdx < 10 ? "0" + std::to_string(oldIdx) : std::to_string(oldIdx);
                    auto initID = controllerRef.getSoloIsSide()
                                      ? sideSolo::ID + oldSuffix
                                      : solo::ID + oldSuffix;
                    parameterRef.getParameter(initID)->beginChangeGesture();
                    parameterRef.getParameter(initID)->setValueNotifyingHost(static_cast<float>(false));
                    parameterRef.getParameter(initID)->endChangeGesture();
                }
                controllerRef.setSolo(idx, isSide);
            } else {
                if (idx == controllerRef.getSoloIdx() && isSide == controllerRef.getSoloIsSide()) {
                    controllerRef.clearSolo();
                }
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

    template<typename FloatType>
    void SoloAttach<FloatType>::initDefaultValues() {
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (size_t j = 0; j < defaultVs.size(); ++j) {
                parameterChanged(initIDs[j] + suffix, defaultVs[j]);
            }
        }
    }

    template
    class SoloAttach<float>;

    template
    class SoloAttach<double>;
}
