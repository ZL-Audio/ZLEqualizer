// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "chore_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    ChoreAttach<FloatType>::ChoreAttach(juce::AudioProcessor &processor,
                                        juce::AudioProcessorValueTreeState &parameters,
                                        juce::AudioProcessorValueTreeState &parametersNA,
                                        Controller<FloatType> &controller)
        : processorRef(processor),
          parameterRef(parameters), parameterNARef(parametersNA),
          controllerRef(controller) {
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    ChoreAttach<FloatType>::~ChoreAttach() {
        for (auto &ID: IDs) {
            parameterRef.removeParameterListener(ID, this);
        }
        for (auto &ID: NAIDs) {
            parameterNARef.removeParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::addListeners() {
        for (auto &ID: IDs) {
            parameterRef.addParameterListener(ID, this);
        }
        for (auto &ID: NAIDs) {
            parameterNARef.addParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == sideChain::ID) {
            controllerRef.setSideChain(static_cast<bool>(newValue));
        } else if (parameterID == zlState::ffTStyle::ID) {
            controllerRef.setFFTStyle(static_cast<zlState::ffTStyle::styles>(newValue));
        } else if (parameterID == zlState::ffTSpeed::ID) {
            const auto idx = static_cast<size_t>(newValue);
            controllerRef.getAnalyzer().getPreFFT().setDecayRate(zlState::ffTSpeed::speeds[idx]);
            controllerRef.getAnalyzer().getPostFFT().setDecayRate(zlState::ffTSpeed::speeds[idx]);
        } else if (parameterID == zlState::ffTTilt::ID) {
            const auto idx = static_cast<size_t>(newValue);
            controllerRef.getAnalyzer().getPreFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
            controllerRef.getAnalyzer().getPostFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::initDefaultValues() {
        for (size_t j = 0; j < defaultVs.size(); ++j) {
            parameterChanged(IDs[j], defaultVs[j]);
        }
    }

    template
    class ChoreAttach<float>;

    template
    class ChoreAttach<double>;
} // zlDSP
