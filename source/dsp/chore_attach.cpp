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
          controllerRef(controller),
          decaySpeed(zlState::ffTSpeed::speeds[static_cast<size_t>(zlState::ffTSpeed::defaultI)]) {
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
        } else if (parameterID == zlState::fftPreON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controllerRef.getAnalyzer().setPreON(false);
                    break;
                case 1:
                    if (isFFTON[0].load() == 0) {
                        controllerRef.getAnalyzer().setPreON(true);
                    }
                    controllerRef.getAnalyzer().getPreFFT().setDecayRate(decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[0].load() == 0) {
                        controllerRef.getAnalyzer().setPreON(true);
                    }
                    controllerRef.getAnalyzer().getPreFFT().setDecayRate(1.f);
                    break;
                default: {
                }
            }
            isFFTON[0].store(static_cast<int>(newValue));
        } else if (parameterID == zlState::fftPostON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controllerRef.getAnalyzer().setPostON(false);
                    break;
                case 1:
                    if (isFFTON[1].load() == 0) {
                        controllerRef.getAnalyzer().setPostON(true);
                    }
                    controllerRef.getAnalyzer().getPostFFT().setDecayRate(decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[1].load() == 0) {
                        controllerRef.getAnalyzer().setPostON(true);
                    }
                    controllerRef.getAnalyzer().getPostFFT().setDecayRate(1.f);
                    break;
                default: {
                }
            }
            isFFTON[1].store(static_cast<int>(newValue));
        } else if (parameterID == zlState::fftSideON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controllerRef.getAnalyzer().setSideON(false);
                    break;
                case 1:
                    if (isFFTON[2].load() == 0) {
                        controllerRef.getAnalyzer().setSideON(true);
                    }
                    controllerRef.getAnalyzer().getSideFFT().setDecayRate(decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[2].load() == 0) {
                        controllerRef.getAnalyzer().setSideON(true);
                    }
                    controllerRef.getAnalyzer().getSideFFT().setDecayRate(1.f);
                    break;
                default: {
                }
            }
            isFFTON[2].store(static_cast<int>(newValue));
        } else if (parameterID == zlState::ffTSpeed::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto speed = zlState::ffTSpeed::speeds[idx];
            decaySpeed.store(speed);
            if (isFFTON[0].load() != 2) controllerRef.getAnalyzer().getPreFFT().setDecayRate(speed);
            if (isFFTON[1].load() != 2) controllerRef.getAnalyzer().getPostFFT().setDecayRate(speed);
            if (isFFTON[2].load() != 2) controllerRef.getAnalyzer().getSideFFT().setDecayRate(speed);
        } else if (parameterID == zlState::ffTTilt::ID) {
            const auto idx = static_cast<size_t>(newValue);
            controllerRef.getAnalyzer().getPreFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
            controllerRef.getAnalyzer().getPostFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
            controllerRef.getAnalyzer().getSideFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
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
