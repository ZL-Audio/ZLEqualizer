// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "chore_attach.hpp"

namespace zlp {
    template<typename FloatType>
    ChoreAttach<FloatType>::ChoreAttach(juce::AudioProcessor &processor,
                                        juce::AudioProcessorValueTreeState &parameters,
                                        juce::AudioProcessorValueTreeState &parametersNA,
                                        Controller<FloatType> &controller)
        : processor_ref(processor),
          parameters_ref(parameters), parameters_NA_ref(parametersNA),
          controller_ref(controller),
          decaySpeed(zlstate::ffTSpeed::speeds[static_cast<size_t>(zlstate::ffTSpeed::defaultI)]) {
        juce::ignoreUnused(parameters_ref);
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    ChoreAttach<FloatType>::~ChoreAttach() {
        for (auto &ID: IDs) {
            parameters_ref.removeParameterListener(ID, this);
        }
        for (auto &ID: NAIDs) {
            parameters_NA_ref.removeParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::addListeners() {
        for (auto &ID: IDs) {
            parameters_ref.addParameterListener(ID, this);
        }
        for (auto &ID: NAIDs) {
            parameters_NA_ref.addParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == sideChain::ID) {
            controller_ref.setSideChain(newValue > .5f);
        } else if (parameterID == dynLookahead::ID) {
            controller_ref.setLookAhead(static_cast<FloatType>(newValue));
        } else if (parameterID == dynRMS::ID) {
            controller_ref.setRMS(static_cast<FloatType>(newValue));
        } else if (parameterID == dynSmooth::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                controller_ref.getFilter(i).getFollower().setSmooth(static_cast<FloatType>(newValue));
            }
        } else if (parameterID == effectON::ID) {
            controller_ref.setEffectON(newValue > .5f);
        } else if (parameterID == phaseFlip::ID) {
            controller_ref.getPhaseFlipper().setON(newValue > .5f);
        } else if (parameterID == staticAutoGain::ID) {
            controller_ref.setSgcON(newValue > .5f);
        } else if (parameterID == autoGain::ID) {
            controller_ref.getAutoGain().enable(newValue > .5f);
        } else if (parameterID == scale::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                auto baseGain = parameters_ref.getRawParameterValue(appendSuffix(gain::ID, i))->load();
                auto targetGain = parameters_ref.getRawParameterValue(appendSuffix(targetGain::ID, i))->load();
                baseGain = zlp::gain::range.snapToLegalValue(baseGain * scale::formatV(newValue));
                targetGain = zlp::targetGain::range.snapToLegalValue(targetGain * scale::formatV(newValue));
                controller_ref.getBaseFilter(i).setGain(baseGain);
                controller_ref.getFilter(i).getMainFilter().setGain(baseGain);
                controller_ref.getMainIIRFilter(i).setGain(baseGain);
                controller_ref.getMainIdealFilter(i).setGain(baseGain);
                controller_ref.getTargetFilter(i).setGain(targetGain);
            }
        } else if (parameterID == outputGain::ID) {
            controller_ref.getGainDSP().setGainDecibels(static_cast<FloatType>(newValue));
        } else if (parameterID == filterStructure::ID) {
            controller_ref.setFilterStructure(static_cast<filterStructure::FilterStructure>(newValue));
        } else if (parameterID == dynHQ::ID) {
            for (size_t i = 0; i < bandNUM; ++i) {
                controller_ref.getFilter(i).setIsPerSample(newValue > .5f);
            }
        } else if (parameterID == zeroLatency::ID) {
            controller_ref.setZeroLatency(newValue > .5f);
        } else if (parameterID == zlstate::fftPreON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controller_ref.getAnalyzer().setPreON(false);
                    break;
                case 1:
                    if (isFFTON[0].load() == 0) {
                        controller_ref.getAnalyzer().setPreON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(0, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[0].load() == 0) {
                        controller_ref.getAnalyzer().setPreON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(0, 1.f);
                    break;
                default: {
                }
            }
            isFFTON[0].store(static_cast<int>(newValue));
        } else if (parameterID == zlstate::fftPostON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controller_ref.getAnalyzer().setPostON(false);
                    break;
                case 1:
                    if (isFFTON[1].load() == 0) {
                        controller_ref.getAnalyzer().setPostON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(1, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[1].load() == 0) {
                        controller_ref.getAnalyzer().setPostON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(1, 1.f);
                    break;
                default: {
                }
            }
            isFFTON[1].store(static_cast<int>(newValue));
        } else if (parameterID == zlstate::fftSideON::ID) {
            switch (static_cast<size_t>(newValue)) {
                case 0:
                    controller_ref.getAnalyzer().setSideON(false);
                    break;
                case 1:
                    if (isFFTON[2].load() == 0) {
                        controller_ref.getAnalyzer().setSideON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(2, decaySpeed.load());
                    break;
                case 2:
                    if (isFFTON[2].load() == 0) {
                        controller_ref.getAnalyzer().setSideON(true);
                    }
                    controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(2, 1.f);
                    break;
                default: {
                }
            }
            isFFTON[2].store(static_cast<int>(newValue));
        } else if (parameterID == zlstate::ffTSpeed::ID) {
            const auto idx = static_cast<size_t>(newValue);
            const auto speed = zlstate::ffTSpeed::speeds[idx];
            decaySpeed.store(speed);
            if (isFFTON[0].load() != 2) controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(0, speed);
            if (isFFTON[1].load() != 2) controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(1, speed);
            if (isFFTON[2].load() != 2) controller_ref.getAnalyzer().getMultipleFFT().setDecayRate(2, speed);
        } else if (parameterID == zlstate::ffTTilt::ID) {
            const auto idx = static_cast<size_t>(newValue);
            controller_ref.getAnalyzer().getMultipleFFT().setTiltSlope(zlstate::ffTTilt::slopes[idx]);
        } else if (parameterID == zlstate::conflictON::ID) {
            const auto f = newValue > .5f;
            controller_ref.getConflictAnalyzer().setON(f);
        } else if (parameterID == zlstate::conflictStrength::ID) {
            controller_ref.getConflictAnalyzer().setStrength(
                zlstate::conflictStrength::formatV(static_cast<FloatType>(newValue)));
        } else if (parameterID == zlstate::conflictScale::ID) {
            controller_ref.getConflictAnalyzer().setConflictScale(static_cast<FloatType>(newValue));
        } else if (parameterID == loudnessMatcherON::ID) {
            if (newValue > .5f) {
                controller_ref.setLoudnessMatcherON(true);
            } else {
                controller_ref.setLoudnessMatcherON(false);
            }
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
} // zldsp
