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
                                        juce::AudioProcessorValueTreeState &parameters_NA,
                                        Controller<FloatType> &controller)
        : processor_ref_(processor),
          parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          controller_ref_(controller),
          decay_speed_(zlstate::ffTSpeed::speeds[static_cast<size_t>(zlstate::ffTSpeed::defaultI)]) {
        juce::ignoreUnused(parameters_ref_);
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    ChoreAttach<FloatType>::~ChoreAttach() {
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
        for (auto &ID: kNAIDs) {
            parameters_NA_ref_.removeParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::addListeners() {
        for (auto &ID: kIDs) {
            parameters_ref_.addParameterListener(ID, this);
        }
        for (auto &ID: kNAIDs) {
            parameters_NA_ref_.addParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == sideChain::ID) {
            controller_ref_.setSideChain(new_value > .5f);
        } else if (parameter_id == dynLookahead::ID) {
            controller_ref_.setLookAhead(static_cast<FloatType>(new_value));
        } else if (parameter_id == dynRMS::ID) {
            controller_ref_.setRMS(static_cast<FloatType>(new_value));
        } else if (parameter_id == dynSmooth::ID) {
            for (size_t i = 0; i < kBandNUM; ++i) {
                controller_ref_.getFilter(i).getFollower().setSmooth(static_cast<FloatType>(new_value));
            }
        } else if (parameter_id == effectON::ID) {
            controller_ref_.setEffectON(new_value > .5f);
        } else if (parameter_id == phaseFlip::ID) {
            controller_ref_.getPhaseFlipper().setON(new_value > .5f);
        } else if (parameter_id == staticAutoGain::ID) {
            controller_ref_.setSgcON(new_value > .5f);
        } else if (parameter_id == autoGain::ID) {
            controller_ref_.getAutoGain().enable(new_value > .5f);
        } else if (parameter_id == scale::ID) {
            for (size_t i = 0; i < kBandNUM; ++i) {
                auto base_gain = parameters_ref_.getRawParameterValue(appendSuffix(gain::ID, i))->load();
                auto target_gain = parameters_ref_.getRawParameterValue(appendSuffix(targetGain::ID, i))->load();
                base_gain = zlp::gain::range.snapToLegalValue(base_gain * scale::formatV(new_value));
                target_gain = zlp::targetGain::range.snapToLegalValue(target_gain * scale::formatV(new_value));
                controller_ref_.getBaseFilter(i).setGain(base_gain);
                controller_ref_.getFilter(i).getMainFilter().setGain(base_gain);
                controller_ref_.getMainIIRFilter(i).setGain(base_gain);
                controller_ref_.getMainIdealFilter(i).setGain(base_gain);
                controller_ref_.getTargetFilter(i).setGain(target_gain);
            }
        } else if (parameter_id == outputGain::ID) {
            controller_ref_.getGainDSP().setGainDecibels(static_cast<FloatType>(new_value));
        } else if (parameter_id == filterStructure::ID) {
            controller_ref_.setFilterStructure(static_cast<filterStructure::FilterStructure>(new_value));
        } else if (parameter_id == dynHQ::ID) {
            for (size_t i = 0; i < kBandNUM; ++i) {
                controller_ref_.getFilter(i).setIsPerSample(new_value > .5f);
            }
        } else if (parameter_id == zeroLatency::ID) {
            controller_ref_.setZeroLatency(new_value > .5f);
        } else if (parameter_id == zlstate::fftPreON::ID) {
            switch (static_cast<size_t>(new_value)) {
                case 0:
                    controller_ref_.getAnalyzer().setPreON(false);
                    break;
                case 1:
                    if (is_fft_on_[0].load() == 0) {
                        controller_ref_.getAnalyzer().setPreON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(0, decay_speed_.load());
                    break;
                case 2:
                    if (is_fft_on_[0].load() == 0) {
                        controller_ref_.getAnalyzer().setPreON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(0, 1.f);
                    break;
                default: {
                }
            }
            is_fft_on_[0].store(static_cast<int>(new_value));
        } else if (parameter_id == zlstate::fftPostON::ID) {
            switch (static_cast<size_t>(new_value)) {
                case 0:
                    controller_ref_.getAnalyzer().setPostON(false);
                    break;
                case 1:
                    if (is_fft_on_[1].load() == 0) {
                        controller_ref_.getAnalyzer().setPostON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(1, decay_speed_.load());
                    break;
                case 2:
                    if (is_fft_on_[1].load() == 0) {
                        controller_ref_.getAnalyzer().setPostON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(1, 1.f);
                    break;
                default: {
                }
            }
            is_fft_on_[1].store(static_cast<int>(new_value));
        } else if (parameter_id == zlstate::fftSideON::ID) {
            switch (static_cast<size_t>(new_value)) {
                case 0:
                    controller_ref_.getAnalyzer().setSideON(false);
                    break;
                case 1:
                    if (is_fft_on_[2].load() == 0) {
                        controller_ref_.getAnalyzer().setSideON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(2, decay_speed_.load());
                    break;
                case 2:
                    if (is_fft_on_[2].load() == 0) {
                        controller_ref_.getAnalyzer().setSideON(true);
                    }
                    controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(2, 1.f);
                    break;
                default: {
                }
            }
            is_fft_on_[2].store(static_cast<int>(new_value));
        } else if (parameter_id == zlstate::ffTSpeed::ID) {
            const auto idx = static_cast<size_t>(new_value);
            const auto speed = zlstate::ffTSpeed::speeds[idx];
            decay_speed_.store(speed);
            if (is_fft_on_[0].load() != 2) controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(0, speed);
            if (is_fft_on_[1].load() != 2) controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(1, speed);
            if (is_fft_on_[2].load() != 2) controller_ref_.getAnalyzer().getMultipleFFT().setDecayRate(2, speed);
        } else if (parameter_id == zlstate::ffTTilt::ID) {
            const auto idx = static_cast<size_t>(new_value);
            controller_ref_.getAnalyzer().getMultipleFFT().setTiltSlope(zlstate::ffTTilt::slopes[idx]);
        } else if (parameter_id == zlstate::conflictON::ID) {
            const auto f = new_value > .5f;
            controller_ref_.getConflictAnalyzer().setON(f);
        } else if (parameter_id == zlstate::conflictStrength::ID) {
            controller_ref_.getConflictAnalyzer().setStrength(
                zlstate::conflictStrength::formatV(static_cast<FloatType>(new_value)));
        } else if (parameter_id == zlstate::conflictScale::ID) {
            controller_ref_.getConflictAnalyzer().setConflictScale(static_cast<FloatType>(new_value));
        } else if (parameter_id == loudnessMatcherON::ID) {
            if (new_value > .5f) {
                controller_ref_.setLoudnessMatcherON(true);
            } else {
                controller_ref_.setLoudnessMatcherON(false);
            }
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::initDefaultValues() {
        for (size_t j = 0; j < kDefaultVs.size(); ++j) {
            parameterChanged(kIDs[j], kDefaultVs[j]);
        }
        for (size_t j = 0; j < kDefaultNAVs.size(); ++j) {
            parameterChanged(kNAIDs[j], kDefaultNAVs[j]);
        }
    }

    template
    class ChoreAttach<float>;

    template
    class ChoreAttach<double>;
} // zldsp
