// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    SoloAttach<FloatType>::SoloAttach(juce::AudioProcessor &processor,
                                      juce::AudioProcessorValueTreeState &parameters,
                                      Controller<FloatType> &controller) : processorRef(processor),
                                                                           parameterRef(parameters),
                                                                           controllerRef(controller) {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            mainSoloUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, solo::ID + suffix);
            sideSoloUpdater[i] = std::make_unique<zlChore::ParaUpdater>(parameters, sideSolo::ID + suffix);
        }
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    SoloAttach<FloatType>::~SoloAttach() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            for (auto &ID: IDs) {
                parameterRef.removeParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::addListeners() {
        for (size_t i = 0; i < bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            for (auto &ID: IDs) {
                parameterRef.addParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(solo::ID) || parameterID.startsWith(sideSolo::ID)) {
            const auto isSide = parameterID.startsWith(sideSolo::ID);
            if (newValue > .5f) {
                if (idx != soloIdx.load() || isSide != soloIsSide.load()) {
                    const auto oldIdx = soloIdx.load();
                    if (soloIsSide.load()) {
                        sideSoloUpdater[oldIdx]->update(0.f);
                    } else {
                        mainSoloUpdater[oldIdx]->update(0.f);
                    }
                    soloIdx.store(idx);
                    soloIsSide.store(isSide);
                }
                controllerRef.setSolo(idx, isSide);
            } else {
                controllerRef.clearSolo(idx, isSide);
            }
        } else {
            if (controllerRef.getSolo() && idx == soloIdx.load()) {
                controllerRef.setSolo(soloIdx.load(), soloIsSide.load());
            }
        }
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
