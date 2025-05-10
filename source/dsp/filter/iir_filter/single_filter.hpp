// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>
#include "../filter_design/filter_design.hpp"
#include "../../chore/chore.hpp"
#include "coeff/martin_coeff.hpp"
#include "iir_base.hpp"
#include "svf_base.hpp"

namespace zldsp::filter {
    /**
     * an IIR filter which processes audio on the real-time thread
     * the maximum modulation rate of parameters is once per block
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class IIR {
    public:
        IIR() = default;

        void reset() {
            if (to_reset_.exchange(false)) {
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    filters_[i].reset();
                }
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    svf_filters_[i].reset();
                }
            }
        }

        void setToRest() { to_reset_.store(true); }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            process_spec_ = spec;
            num_channels_.store(spec.numChannels);
            for (auto &f: filters_) {
                f.prepare(spec);
            }
            for (auto &f: svf_filters_) {
                f.prepare(spec);
            }
            setOrder(order_.load());
            parallel_buffer_.setSize(static_cast<int>(spec.numChannels),
                                   static_cast<int>(spec.maximumBlockSize));
            c_freq_.prepare(spec.sampleRate, 0.1);
            c_gain_.prepare(spec.sampleRate, 0.001);
            c_q_.prepare(spec.sampleRate, 0.001);
        }

        /**
         * prepare for processing the incoming audio buffer
         * call it when you want to update filter parameters
         * @param buffer
         */
        void processPre(juce::AudioBuffer<FloatType> &buffer) {
            if (c_filter_structure_ != filter_structure_.load() || current_filter_type_ != filter_type_.load()) {
                c_filter_structure_ = filter_structure_.load();
                current_filter_type_ = filter_type_.load();
                should_be_parallel_ = (current_filter_type_ == FilterType::kPeak) || (
                                       current_filter_type_ == FilterType::kLowShelf) || (
                                       current_filter_type_ == FilterType::kHighShelf) || (
                                       current_filter_type_ == FilterType::kBandShelf);
                should_not_be_parallel_ = !should_be_parallel_;
                should_be_parallel_ = should_be_parallel_ && (c_filter_structure_ == FilterStructure::kParallel);
                should_not_be_parallel_ = should_not_be_parallel_ && (c_filter_structure_ == FilterStructure::kParallel);
                to_reset_.store(true);
                updateCoeffs();
            }
            if (should_be_parallel_) {
                parallel_buffer_.makeCopyOf(buffer);
            }
            reset();
            if (to_update_para_.exchange(false)) {
                updateCoeffs();
            }
            if (to_update_fgq_.exchange(false)) {
                c_freq_.setTarget(freq_.load());
                c_gain_.setTarget(gain_.load());
                c_q_.setTarget(q_.load());
            }
        }

        /**
         * process the incoming audio buffer
         * for parallel filter, call it with the internal parallel buffer
         * @param buffer
         */
        template<bool IsBypassed = false>
        void process(juce::AudioBuffer<FloatType> &buffer) {
            const auto f1 = c_freq_.isSmoothing();
            const auto f2 = c_gain_.isSmoothing();
            const auto f3 = c_q_.isSmoothing();
            switch (c_filter_structure_) {
                case FilterStructure::kIIR: {
                    if (f1 || f2 || f3) {
                        processIIR<IsBypassed, true>(buffer);
                    } else {
                        processIIR<IsBypassed, false>(buffer);
                    }
                    break;
                }
                case FilterStructure::kSVF: {
                    if (f1 || f2 || f3) {
                        processSVF<IsBypassed, true>(buffer);
                    } else {
                        processSVF<IsBypassed, false>(buffer);
                    }
                    break;
                }
                case FilterStructure::kParallel: {
                    if (should_be_parallel_) {
                        auto block = juce::dsp::AudioBlock<FloatType>(buffer);
                        auto context = juce::dsp::ProcessContextReplacing<FloatType>(block);
                        context.isBypassed = IsBypassed;
                        if (f2) {
                            const auto multiplier0 = parallel_multiplier_;
                            if (f1 || f3) {
                                processIIR<IsBypassed, true>(buffer);
                            } else {
                                processIIR<IsBypassed, false>(buffer);
                                skipSmooth();
                            }
                            const auto multiplier2 = parallel_multiplier_;
                            const auto w = static_cast<FloatType>(buffer.getNumSamples() - 1) / static_cast<FloatType>(
                                               buffer.getNumSamples());
                            const auto multiplier1 = multiplier0 * w + multiplier2 * (FloatType(1) - w);
                            buffer.applyGainRamp(0, buffer.getNumSamples(), multiplier1, multiplier2);
                        } else {
                            if (f1 || f3) {
                                processIIR<IsBypassed, true>(buffer);
                            } else {
                                processIIR<IsBypassed, false>(buffer);
                            }
                            buffer.applyGain(parallel_multiplier_);
                        }
                        break;
                    } else {
                        if (f1 || f2 || f3) {
                            processIIR<IsBypassed, true>(buffer);
                        } else {
                            processIIR<IsBypassed, false>(buffer);
                        }
                        break;
                    }
                }
            }
        }

        template<bool IsBypassed = false, bool IsSmooth = false>
        void processIIR(juce::AudioBuffer<FloatType> &buffer) {
            const auto writerPointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                if (IsSmooth) updateCoeffs();
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writerPointer[channel] + i);
                    for (size_t filter_idx = 0; filter_idx < current_filter_num_; ++filter_idx) {
                        sample = filters_[filter_idx].processSample(static_cast<size_t>(channel), sample);
                    }
                    if (!IsBypassed) {
                        *(writerPointer[channel] + i) = sample;
                    }
                }
            }
        }

        template<bool IsBypassed = false, bool IsSmooth = false>
        void processSVF(juce::AudioBuffer<FloatType> &buffer) {
            const auto writerPointer = buffer.getArrayOfWritePointers();
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                if (IsSmooth) updateCoeffs();
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto sample = *(writerPointer[static_cast<size_t>(channel)] + i);
                    for (size_t filter_idx = 0; filter_idx < current_filter_num_; ++filter_idx) {
                        sample = svf_filters_[filter_idx].processSample(static_cast<size_t>(channel), sample);
                    }
                    if (!IsBypassed) {
                        *(writerPointer[static_cast<size_t>(channel)] + i) = sample;
                    }
                }
            }
        }

        /**
         * add the processed parallel buffer to the incoming audio buffer
         * @param buffer
         */
        template<bool IsBypassed = false>
        void processParallelPost(juce::AudioBuffer<FloatType> &buffer) {
            if (IsBypassed) return;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                auto *dest = buffer.getWritePointer(channel);
                auto *source = parallel_buffer_.getWritePointer(channel);
                for (size_t idx = 0; idx < static_cast<size_t>(buffer.getNumSamples()); ++idx) {
                    dest[idx] = dest[idx] + source[idx];
                }
            }
        }

        /**
         * set the frequency of the filter
         * @param x frequency
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setFreq(const FloatType x) {
            if (Async) {
                freq_.store(static_cast<double>(x));
                if (Update) { to_update_fgq_.store(true); }
            } else {
                if (Force) {
                    c_freq_.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    c_freq_.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool Async = true>
        FloatType getFreq() const {
            if (Async) {
                return static_cast<FloatType>(freq_.load());
            } else {
                return static_cast<FloatType>(c_freq_.getCurrent());
            }
        }

        /**
         * set the gain of the filter
         * @param x gain
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setGain(const FloatType x) {
            if (Async) {
                gain_.store(static_cast<double>(x));
                if (Update) to_update_fgq_.store(true);
            } else {
                if (Force) {
                    c_gain_.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    c_gain_.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool Async = true>
        FloatType getGain() const {
            if (Async) {
                return static_cast<FloatType>(gain_.load());
            } else {
                return static_cast<FloatType>(c_gain_.getCurrent());
            }
        }

        /**
         * set the Q value of the filter
         * @param x Q value
         */
        template<bool Update = true, bool Async = true, bool Force = false>
        void setQ(const FloatType x) {
            if (Async) {
                q_.store(static_cast<double>(x));
                if (Update) to_update_fgq_.store(true);
            } else {
                if (Force) {
                    c_q_.setCurrentAndTarget(static_cast<double>(x));
                } else {
                    c_q_.setTarget(static_cast<double>(x));
                }
            }
        }

        template<bool Async = true>
        FloatType getQ() const {
            if (Async) {
                return static_cast<FloatType>(q_.load());
            } else {
                return static_cast<FloatType>(c_q_.getCurrent());
            }
        }

        void skipSmooth() {
            c_freq_.setCurrentAndTarget(c_freq_.getTarget());
            c_gain_.setCurrentAndTarget(c_gain_.getTarget());
            c_q_.setCurrentAndTarget(c_q_.getTarget());
            updateCoeffs();
        }

        /**
         * set the type of the filter, the filter will always reset
         * @param x filter type
         */
        template<bool Update = true>
        void setFilterType(const FilterType x) {
            to_reset_.store(true);
            filter_type_.store(x);
            if (Update) { to_update_para_.store(true); }
        }

        inline FilterType getFilterType() const { return filter_type_.load(); }

        /**
         * set the order of the filter, the filter will always reset
         * @param x filter order
         */
        template<bool Update = true>
        void setOrder(const size_t x) {
            order_.store(x);
            if (Update) {
                to_reset_.store(true);
                to_update_para_.store(true);
            }
        }

        inline size_t getOrder() const { return order_.load(); }

        /**
         * update filter coefficients
         * DO NOT call it unless you are sure what you are doing
         */
        void updateCoeffs() {
            const auto next_freq = c_freq_.getNext();
            const auto next_gain = c_gain_.getNext();
            const auto next_q = c_q_.getNext();
            if (!should_be_parallel_) {
                current_filter_num_ = updateIIRCoeffs(current_filter_type_, order_.load(),
                                                   next_freq, process_spec_.sampleRate,
                                                   next_gain, next_q, coeffs_);
            } else {
                if (current_filter_type_ == FilterType::kPeak) {
                    current_filter_num_ = updateIIRCoeffs(FilterType::kBandPass,
                                                       std::min(static_cast<size_t>(4), order_.load()),
                                                       next_freq, process_spec_.sampleRate,
                                                       next_gain, next_q, coeffs_);
                } else if (current_filter_type_ == FilterType::kLowShelf) {
                    current_filter_num_ = updateIIRCoeffs(FilterType::kLowPass,
                                                       std::min(static_cast<size_t>(2), order_.load()),
                                                       next_freq, process_spec_.sampleRate,
                                                       next_gain, next_q, coeffs_);
                } else if (current_filter_type_ == FilterType::kHighShelf) {
                    current_filter_num_ = updateIIRCoeffs(FilterType::kHighPass,
                                                       std::min(static_cast<size_t>(2), order_.load()),
                                                       next_freq, process_spec_.sampleRate,
                                                       next_gain, next_q, coeffs_);
                }
                updateParallelGain(next_gain);
            }
            switch (c_filter_structure_) {
                case FilterStructure::kIIR:
                case FilterStructure::kParallel: {
                    for (size_t i = 0; i < current_filter_num_; i++) {
                        filters_[i].updateFromBiquad(coeffs_[i]);
                    }
                    break;
                }
                case FilterStructure::kSVF: {
                    for (size_t i = 0; i < current_filter_num_; i++) {
                        svf_filters_[i].updateFromBiquad(coeffs_[i]);
                    }
                }
            }
        }

        /**
         * get the num of channels
         * @return
         */
        inline juce::uint32 getNumChannels() const { return num_channels_.load(); }

        /**
         * get the array of 2nd order filters
         * @return
         */
        std::array<IIRBase<FloatType>, FilterSize> &getFilters() { return filters_; }

        void setFilterStructure(const FilterStructure x) {
            filter_structure_.store(x);
        }

        bool getShouldBeParallel() const { return should_be_parallel_; }

        bool getShouldNotBeParallel() const { return should_not_be_parallel_; }

        juce::AudioBuffer<FloatType> &getParallelBuffer() { return parallel_buffer_; }

    private:
        std::array<IIRBase<FloatType>, FilterSize> filters_{};
        juce::AudioBuffer<FloatType> parallel_buffer_;

        size_t current_filter_num_{1};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::Lin> c_gain_{0.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::Mul> c_q_{0.707};
        zldsp::chore::SmoothedValue<double, zldsp::chore::FixMul> c_freq_{1000.0};
        std::atomic<size_t> order_{2};
        std::atomic<FilterType> filter_type_{FilterType::kPeak};
        FilterType current_filter_type_{FilterType::kPeak};

        juce::dsp::ProcessSpec process_spec_{48000, 512, 2};
        std::atomic<juce::uint32> num_channels_{2};

        std::atomic<bool> to_update_para_{true}, to_reset_{true};
        std::atomic<bool> to_update_fgq_{false};

        std::array<std::array<double, 6>, FilterSize> coeffs_{};

        std::atomic<bool> use_svf_{false};
        bool c_use_svf_{false};
        std::array<SVFBase<FloatType>, FilterSize> svf_filters_{};

        std::atomic<FilterStructure> filter_structure_{FilterStructure::kIIR};
        FilterStructure c_filter_structure_{FilterStructure::kIIR};
        bool should_be_parallel_{false}, should_not_be_parallel_{false};
        FloatType parallel_multiplier_;

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, FilterSize> &coeffs) {
            return FilterDesign::updateCoeffs<FilterSize,
                MartinCoeff::get1LowShelf, MartinCoeff::get1HighShelf, MartinCoeff::get1TiltShelf,
                MartinCoeff::get1LowPass, MartinCoeff::get1HighPass,
                MartinCoeff::get2Peak,
                MartinCoeff::get2LowShelf, MartinCoeff::get2HighShelf, MartinCoeff::get2TiltShelf,
                MartinCoeff::get2LowPass, MartinCoeff::get2HighPass,
                MartinCoeff::get2BandPass, MartinCoeff::get2Notch>(
                filterType, n, f, fs, g0, q0, coeffs);
        }

        void updateParallelGain(double x) {
            parallel_multiplier_ = juce::Decibels::decibelsToGain<FloatType>(static_cast<FloatType>(x)) - FloatType(1);
        }
    };
}
