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

#include "../iir_filter/iir_filter.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "../../compressor/compressor.hpp"

namespace zldsp::filter {
    /**
     * a dynamic IIR filter which holds a main filter, a base filter, a target filter and a side filter
     * the output signal is filtered by the main filter, whose gain and Q is set by the mix of base/target filters'
     * the mix portion is controlled by a compressor on the signal from the side filter (on the side chain)
     * @tparam FloatType
     */
    template<typename FloatType, size_t FilterSize>
    class DynamicIIR {
    public:
        DynamicIIR(zldsp::filter::Empty<FloatType> &b, zldsp::filter::Empty<FloatType> &t)
            : b_filter_(b), t_filter_(t) {
        }

        void reset() {
            m_filter_.reset();
            s_filter_.reset();
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            m_filter_.prepare(spec);
            s_filter_.template setOrder<false>(2);
            s_filter_.template setFilterType<false>(zldsp::filter::FilterType::kBandPass);
            s_filter_.prepare(spec);
            const auto sr = spec.sampleRate / static_cast<double>(spec.maximumBlockSize);
            follower_.prepare(sr);
            tracker_.prepare(sr);

            computer_.setRatio(FloatType(1000));
            s_buffer_copy_.setSize(static_cast<int>(spec.numChannels),
                                static_cast<int>(spec.maximumBlockSize));
        }

        /**
         * process the audio buffer
         * @param mBuffer main chain audio buffer
         * @param sBuffer side chain audio buffer
         */
        template<bool isBypassed = false>
        void process(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            cacheCurrentValues();
            m_filter_.processPre(mBuffer);
            if (c_dynamic_on_) {
                if (!m_filter_.getShouldNotBeParallel()) {
                    processDynamic<isBypassed>(mBuffer, sBuffer);
                }
            } else {
                if (m_filter_.getShouldBeParallel()) {
                    m_filter_.template process<isBypassed>(m_filter_.getParallelBuffer());
                } else if (!m_filter_.getShouldNotBeParallel()) {
                    m_filter_.template process<isBypassed>(mBuffer);
                }
            }
        }

        template<bool isBypassed = false>
        void processParallelPost(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            if (m_filter_.getShouldNotBeParallel()) {
                if (c_dynamic_on_) {
                    processDynamic<isBypassed>(mBuffer, sBuffer);
                } else {
                    m_filter_.template process<isBypassed>(mBuffer);
                }
            } else if (m_filter_.getShouldBeParallel()) {
                m_filter_.template processParallelPost<isBypassed>(mBuffer);
            }
        }

        static void processBypass() {
        }

        IIR<FloatType, FilterSize> &getMainFilter() { return m_filter_; }

        IIR<FloatType, FilterSize> &getSideFilter() { return s_filter_; }

        zldsp::compressor::KneeComputer<FloatType, false, false> &getComputer() { return computer_; }

        zldsp::compressor::RMSTracker<FloatType, false> &getTracker() { return tracker_; }

        zldsp::compressor::PSFollower<FloatType, true, false> &getFollower() { return follower_; }

        void setActive(const bool x) {
            if (x) {
                m_filter_.setToRest();
            }
            active_.store(x);
        }

        void setDynamicON(const bool x) { dynamic_on_.store(x); }

        bool getDynamicON() const { return dynamic_on_.load(); }

        void setDynamicBypass(const bool x) { dynamic_bypass_.store(x); }

        bool getDynamicBypass() const { return dynamic_bypass_.load(); }

        void setFilterStructure(const FilterStructure x) {
            filter_structure_.store(x);
        }

        void setIsPerSample(const bool x) { is_per_sample_.store(x); }

        void setBaseLine(const FloatType x) { comp_baseline_ = x; }

        FloatType getBaseLine() const { return comp_baseline_; }

    private:
        zldsp::filter::IIR<FloatType, FilterSize> m_filter_, s_filter_;
        zldsp::filter::Empty<FloatType> &b_filter_, &t_filter_;
        zldsp::compressor::KneeComputer<FloatType, false, false> computer_;
        zldsp::compressor::RMSTracker<FloatType, false> tracker_;
        zldsp::compressor::PSFollower<FloatType, true, false> follower_;
        FloatType comp_baseline_;
        juce::AudioBuffer<FloatType> s_buffer_copy_;
        std::atomic<bool> active_{false}, dynamic_on_{false}, dynamic_bypass_{false};
        bool c_dynamic_on_{false}, c_dynamic_bypass_{false};
        std::atomic<FilterStructure> filter_structure_{FilterStructure::kIIR};
        FilterStructure current_filter_structure_{FilterStructure::kIIR};
        juce::AudioBuffer<FloatType> sample_buffer_;
        std::atomic<bool> is_per_sample_{false};
        bool c_is_per_sample_{false};

        template<bool IsBypassed = false>
        void processDynamic(juce::AudioBuffer<FloatType> &mBuffer, juce::AudioBuffer<FloatType> &sBuffer) {
            s_buffer_copy_.makeCopyOf(sBuffer, true);
            s_filter_.processPre(s_buffer_copy_);
            s_filter_.process(s_buffer_copy_);
            // feed side-chain into the tracker
            tracker_.processBufferRMS(s_buffer_copy_);
            // get loudness from tracker
            const auto currentLoudness = tracker_.getMomentaryLoudness() - comp_baseline_;
            // calculate the reduction
            const auto reducedLoudness = currentLoudness - computer_.eval(currentLoudness);
            // calculate gain/Q mix portion
            auto portion = std::min(reducedLoudness / computer_.getReductionAtKnee(), FloatType(1));
            // smooth the portion
            portion = follower_.processSample(portion);
            if (c_dynamic_bypass_) {
                portion = 0;
            }
            if (c_is_per_sample_) {
                m_filter_.template setGain<true, false, false>(
                    (1 - portion) * b_filter_.getGain() + portion * t_filter_.getGain());
                m_filter_.template setQ<true, false, false>((1 - portion) * b_filter_.getQ() + portion * t_filter_.getQ());
            } else {
                m_filter_.template setGain<true, false, true>(
                    (1 - portion) * b_filter_.getGain() + portion * t_filter_.getGain());
                m_filter_.template setQ<true, false, true>((1 - portion) * b_filter_.getQ() + portion * t_filter_.getQ());
                m_filter_.updateCoeffs();
            }
            if (m_filter_.getShouldBeParallel()) {
                m_filter_.template process<IsBypassed>(m_filter_.getParallelBuffer());
            } else {
                m_filter_.template process<IsBypassed>(mBuffer);
            }
        }

        void cacheCurrentValues() {
            if (current_filter_structure_ != filter_structure_.load()) {
                current_filter_structure_ = filter_structure_.load();
                switch (current_filter_structure_) {
                    case FilterStructure::kIIR:
                    case FilterStructure::kSVF: {
                        m_filter_.setFilterStructure(current_filter_structure_);
                        s_filter_.setFilterStructure(current_filter_structure_);
                        break;
                    }
                    case FilterStructure::kParallel: {
                        m_filter_.setFilterStructure(current_filter_structure_);
                        s_filter_.setFilterStructure(FilterStructure::kIIR);
                    }
                }
            }
            c_dynamic_on_ = dynamic_on_.load();
            if (c_dynamic_on_) {
                c_dynamic_bypass_ = dynamic_bypass_.load();
                c_is_per_sample_ = is_per_sample_.load();
                computer_.prepareBuffer();
                tracker_.prepareBuffer();
                follower_.prepareBuffer();
            }
        }
    };
}
