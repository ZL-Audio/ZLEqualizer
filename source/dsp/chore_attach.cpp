// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
        juce::ignoreUnused(parameterRef);
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
            controllerRef.setSideChain(newValue > .5f);
        } else if (parameterID == dynLookahead::ID) {
            controllerRef.setLookAhead(static_cast<FloatType>(newValue));
        } else if (parameterID == dynRMS::ID) {
            controllerRef.setRMS(static_cast<FloatType>(newValue));
        } else if (parameterID == dynSmooth::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                controllerRef.getFilter(i).getCompressor().getDetector().setSmooth(static_cast<FloatType>(newValue));
            }
        } else if (parameterID == effectON::ID) {
            controllerRef.setEffectON(newValue > .5f);
        } else if (parameterID == phaseFlip::ID) {
            controllerRef.getPhaseFlipper().setON(newValue > .5f);
        }else if (parameterID == staticAutoGain::ID) {
            controllerRef.setSgcON(newValue > .5f);
        } else if (parameterID == autoGain::ID) {
            controllerRef.getAutoGain().enable(newValue > .5f);
        } else if (parameterID == scale::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                auto baseGain = parameterRef.getRawParameterValue(appendSuffix(gain::ID, i))->load();
                auto targetGain = parameterRef.getRawParameterValue(appendSuffix(targetGain::ID, i))->load();
                baseGain = zlDSP::gain::range.snapToLegalValue(baseGain * scale::formatV(newValue));
                targetGain = zlDSP::targetGain::range.snapToLegalValue(targetGain * scale::formatV(newValue));
                controllerRef.getBaseFilter(i).setGain(baseGain);
                controllerRef.getFilter(i).getMainFilter().setGain(baseGain);
                controllerRef.getMainIIRFilter(i).setGain(baseGain);
                controllerRef.getMainIdealFilter(i).setGain(baseGain);
                controllerRef.getTargetFilter(i).setGain(targetGain);
            }
        } else if (parameterID == outputGain::ID) {
            controllerRef.getGainDSP().setGainDecibels(static_cast<FloatType>(newValue));
        } else if (parameterID == filterStructure::ID) {
            controllerRef.setFilterStructure(static_cast<filterStructure::FilterStructure>(newValue));
        } else if (parameterID == dynHQ::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                controllerRef.getFilter(i).setIsPerSample(newValue > .5f);
            }
        } else if (parameterID == zeroLatency::ID) {
            controllerRef.setZeroLatency(newValue > .5f);
        } else if (parameterID == zlState::fftPreON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controllerRef.getAnalyzer().setPreON(false);
                    break;
                case 1:
                    if (isFFTON[0].load() == 0) {
                        controllerRef.getAnalyzer().setPreON(true);
                    }
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(0, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[0].load() == 0) {
                        controllerRef.getAnalyzer().setPreON(true);
                    }
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(0, 1.f);
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
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(1, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[1].load() == 0) {
                        controllerRef.getAnalyzer().setPostON(true);
                    }
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(1, 1.f);
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
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(2, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[2].load() == 0) {
                        controllerRef.getAnalyzer().setSideON(true);
                    }
                    controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(2, 1.f);
                    break;
                default: {
                }
            }
            isFFTON[2].store(static_cast<int>(newValue));
        } else if (parameterID == zlState::ffTSpeed::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto speed = zlState::ffTSpeed::speeds[idx];
            decaySpeed.store(speed);
            if (isFFTON[0].load() != 2) controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(0, speed);
            if (isFFTON[1].load() != 2) controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(1, speed);
            if (isFFTON[2].load() != 2) controllerRef.getAnalyzer().getMultipleFFT().setDecayRate(2, speed);
        } else if (parameterID == zlState::ffTTilt::ID) {
            const auto idx = static_cast<size_t>(newValue);
            controllerRef.getAnalyzer().getMultipleFFT().setTiltSlope(zlState::ffTTilt::slopes[idx]);
        } else if (parameterID == zlState::conflictON::ID) {
            const auto f = newValue > .5f;
            controllerRef.getConflictAnalyzer().setON(f);
        } else if (parameterID == zlState::conflictStrength::ID) {
            controllerRef.getConflictAnalyzer().setStrength(
                zlState::conflictStrength::formatV(static_cast<FloatType>(newValue)));
        } else if (parameterID == zlState::conflictScale::ID) {
            controllerRef.getConflictAnalyzer().setConflictScale(static_cast<FloatType>(newValue));
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::initDefaultValues() {
        for (size_t j = 0; j < defaultVs.size(); ++j) {
            parameterChanged(IDs[j], defaultVs[j]);
        }
        for (size_t j = 0; j < defaultNAVs.size(); ++j) {
            parameterChanged(NAIDs[j], defaultNAVs[j]);
        }
    }

    template
    class ChoreAttach<float>;

    template
    class ChoreAttach<double>;
} // zlDSP
