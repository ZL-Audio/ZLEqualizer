// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlp {
    template<typename FloatType>
    Controller<FloatType>::Controller(juce::AudioProcessor &processor, const size_t fft_order)
        : processor_ref_(processor),
          fft_analyzer_(fft_order), conflict_analyzer_(fft_order), match_analyzer_(13) {
        for (size_t i = 0; i < kBandNUM; ++i) {
            histograms_[i].setDecayRate(FloatType(0.99999));
            sub_histograms_[i].setDecayRate(FloatType(0.9995));
        }
        solo_filter_.setFilterStructure(zldsp::filter::FilterStructure::kSVF);
    }

    template<typename FloatType>
    void Controller<FloatType>::reset() {
        for (auto &f: filters_) {
            f.reset();
        }
        solo_filter_.reset();
    }

    template<typename FloatType>
    void Controller<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        delay_.setMaximumDelayInSamples(
            static_cast<int>(zlp::dynLookahead::range.end / 1000.f * static_cast<float>(spec.sampleRate)) + 1);
        delay_.prepare({spec.sampleRate, spec.maximumBlockSize, 2});

        sub_buffer_.prepare({spec.sampleRate, spec.maximumBlockSize, 4});
        sample_rate_.store(spec.sampleRate);
        updateSubBuffer();
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSubBuffer() {
        sub_buffer_.setSubBufferSize(static_cast<int>(kSubBufferLength * sample_rate_.load()));

        for (auto &f: filters_) {
            f.getTracker().setMaximumMomentarySeconds(static_cast<FloatType>(zlp::dynRMS::range.end / 1000.f));
        }

        juce::dsp::ProcessSpec subSpec{sample_rate_.load(), sub_buffer_.getSubSpec().maximumBlockSize, 2};
        for (auto &f: filters_) {
            f.prepare(subSpec);
        }

        prototype_corrections_[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            prototype_corrections_[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        prototype_w1_.resize(prototype_corrections_[0].getCorrectionSize());
        prototype_w2_.resize(prototype_corrections_[0].getCorrectionSize());
        zldsp::filter::calculateWsForPrototype<FloatType>(prototype_w1_);
        zldsp::filter::calculateWsForBiquad<FloatType>(prototype_w2_);

        mixed_corrections_[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            mixed_corrections_[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        mixed_w1_.resize(mixed_corrections_[0].getCorrectionSize());
        mixed_w2_.resize(mixed_corrections_[0].getCorrectionSize());
        zldsp::filter::calculateWsForPrototype<FloatType>(mixed_w1_);
        zldsp::filter::calculateWsForBiquad<FloatType>(mixed_w2_);

        linear_filters_[0].prepare(subSpec);
        for (size_t i = 1; i < 5; ++i) {
            linear_filters_[i].prepare(juce::dsp::ProcessSpec{subSpec.sampleRate, subSpec.maximumBlockSize, 1});
        }
        linear_w1_.resize(linear_filters_[0].getCorrectionSize());
        zldsp::filter::calculateWsForPrototype<FloatType>(linear_w1_);

        for (auto &f: main_IIRs_) {
            f.prepare(subSpec.sampleRate);
            f.prepareResponseSize(mixed_corrections_[0].getCorrectionSize());
        }
        for (auto &f: main_ideals_) {
            f.prepare(subSpec.sampleRate);
            f.prepareResponseSize(linear_filters_[0].getCorrectionSize());
        }

        solo_filter_.setFilterType(zldsp::filter::FilterType::kBandPass);
        solo_filter_.prepare(subSpec);

        lr_main_splitter_.prepare(subSpec);
        lr_side_splitter_.prepare(subSpec);
        ms_main_splitter_.prepare(subSpec);
        ms_side_splitter_.prepare(subSpec);
        output_gain_.prepare(subSpec);
        auto_gain_.prepare(subSpec);
        for (auto &g: compensation_gains_) {
            g.prepare(subSpec);
        }
        fft_analyzer_.prepare(subSpec);
        conflict_analyzer_.prepare(subSpec);
        match_analyzer_.prepare(subSpec);
        dummy_main_buffer_.setSize(static_cast<int>(subSpec.numChannels), static_cast<int>(subSpec.maximumBlockSize));
        dummy_main_delay_.setMaximumDelayInSamples(linear_filters_[0].getLatency() * 3 + 10);
        dummy_main_delay_.prepare(subSpec);
        dummy_side_buffer_.setSize(static_cast<int>(subSpec.numChannels), static_cast<int>(subSpec.maximumBlockSize));
        dummy_side_delay_.setMaximumDelayInSamples(linear_filters_[0].getLatency() * 3 + 10);
        dummy_side_delay_.prepare(subSpec);

        loudness_matcher_.prepare(subSpec);

        for (auto &t: trackers_) {
            t.prepare(subSpec.sampleRate);
        }

        to_update_lrs_.store(true);
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        c_is_editor_on_ = is_editor_on_.load();
        if (filter_structure_.load() != c_filter_structure_) {
            c_filter_structure_ = filter_structure_.load();
            updateFilterStructure();
            to_update_lrs_.store(true);
        }
        if (to_update_dynamic_on_.exchange(false)) {
            updateDynamicONs();
        }
        if (to_update_lrs_.exchange(false)) {
            updateLRs();
            updateTrackersON();
            updateCorrections();
            to_update_sgc_.store(true);
        }
        if (to_update_bypass_.exchange(false)) {
            for (size_t i = 0; i < kBandNUM; ++i) {
                c_is_bypass_[i] = is_bypass_[i].load();
            }
            updateCorrections();
            to_update_sgc_.store(true);
        }
        if (c_is_sgc_on_ != is_sgc_on_.load()) {
            c_is_sgc_on_ = is_sgc_on_.load();
            if (!c_is_sgc_on_) {
                for (auto &cG: compensation_gains_) {
                    cG.setGainLinear(FloatType(1));
                }
            } else {
                to_update_sgc_.store(true);
            }
        }
        if (c_is_sgc_on_) {
            if (to_update_sgc_.exchange(false)) {
                updateSgcValues();
            }
        }
        if (to_update_solo_.exchange(false)) {
            c_use_solo_ = use_solo_.load();
            updateSolo();
        }
        if (to_update_dyn_rel_side_.exchange(false)) {
            updateDynRelSide();
            updateTrackersON();
        }
        if (to_update_hist_.exchange(false)) {
            updateHistograms();
        }
        if (c_is_loudness_matcher_on_ != is_loudness_matcher_on_.load()) {
            c_is_loudness_matcher_on_ = is_loudness_matcher_on_.load();
            if (c_is_loudness_matcher_on_) {
                loudness_matcher_.reset();
            }
        }

        c_is_effect_on_ = is_effect_on_.load();

        juce::AudioBuffer<FloatType> main_buffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<FloatType> side_buffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        // if no side chain, copy the main buffer into the side buffer
        if (!side_chain_.load()) {
            side_buffer.makeCopyOf(main_buffer, true);
        }
        // process lookahead
        delay_.process(main_buffer);
        if (is_zero_latency_.load()) {
            int start_sample = 0;
            const int sample_per_buffer = static_cast<int>(sub_buffer_.getSubSpec().maximumBlockSize);
            while (start_sample < buffer.getNumSamples()) {
                const int actual_num_sample = std::min(sample_per_buffer, buffer.getNumSamples() - start_sample);
                auto sub_main_buffer = juce::AudioBuffer<FloatType>(main_buffer.getArrayOfWritePointers(),
                                                                  2, start_sample, actual_num_sample);
                auto sub_side_buffer = juce::AudioBuffer<FloatType>(side_buffer.getArrayOfWritePointers(),
                                                                  2, start_sample, actual_num_sample);
                processSubBuffer(sub_main_buffer, sub_side_buffer);
                start_sample += sample_per_buffer;
            }
        } else {
            auto block = juce::dsp::AudioBlock<FloatType>(buffer);
            // ---------------- start sub buffer
            sub_buffer_.pushBlock(block);
            while (sub_buffer_.isSubReady()) {
                sub_buffer_.popSubBuffer();
                // create main sub buffer and side sub buffer
                auto sub_main_buffer = juce::AudioBuffer<FloatType>(sub_buffer_.sub_buffer_.getArrayOfWritePointers() + 0,
                                                                  2, sub_buffer_.sub_buffer_.getNumSamples());
                auto sub_side_buffer = juce::AudioBuffer<FloatType>(sub_buffer_.sub_buffer_.getArrayOfWritePointers() + 2,
                                                                  2, sub_buffer_.sub_buffer_.getNumSamples());
                processSubBuffer(sub_main_buffer, sub_side_buffer);
                sub_buffer_.pushSubBuffer();
            }
            sub_buffer_.popBlock(block);
            // ---------------- end sub buffer
        }
        phase_flipper_.process(main_buffer);
    }

    template<typename FloatType>
    void Controller<FloatType>::processSubBuffer(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                                 juce::AudioBuffer<FloatType> &sub_side_buffer) {
        if (c_is_effect_on_) {
            if (c_use_solo_) {
                processSubBufferOnOff<true>(sub_main_buffer, sub_side_buffer);
                processSolo(sub_main_buffer, sub_side_buffer);
            } else {
                processSubBufferOnOff<false>(sub_main_buffer, sub_side_buffer);
            }
        } else {
            processSubBufferOnOff<true>(sub_main_buffer, sub_side_buffer);
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processSubBufferOnOff(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                                      juce::AudioBuffer<FloatType> &sub_side_buffer) {
        dummy_main_buffer_.makeCopyOf(sub_main_buffer, true);
        dummy_main_delay_.process(dummy_main_buffer_);
        if (c_is_editor_on_) {
            dummy_side_buffer_.makeCopyOf(sub_side_buffer, true);
            dummy_side_delay_.process(dummy_side_buffer_);
            match_analyzer_.process(sub_main_buffer, sub_side_buffer);
        }
        if (c_filter_structure_ == filterStructure::kLinear) {
            processLinear<IsBypassed>(sub_main_buffer);
        } else {
            processDynamic<IsBypassed>(sub_main_buffer, sub_side_buffer);
            if (c_filter_structure_ == filterStructure::kParallel) {
                processParallelPost<IsBypassed>(sub_main_buffer, sub_side_buffer);
            }
            if (c_filter_structure_ == filterStructure::kMatched) {
                processPrototypeCorrection<IsBypassed>(sub_main_buffer);
            } else if (c_filter_structure_ == filterStructure::kMixed) {
                processMixedCorrection<IsBypassed>(sub_main_buffer);
            }
        }
        if (c_is_loudness_matcher_on_) {
            loudness_matcher_.process(dummy_main_buffer_, sub_main_buffer);
        }
        auto_gain_.processPre(dummy_main_buffer_);
        auto_gain_.template processPost<IsBypassed>(sub_main_buffer);
        output_gain_.template process<IsBypassed>(sub_main_buffer);
        if (c_is_editor_on_) {
            fft_analyzer_.prepareBuffer();
            fft_analyzer_.process(dummy_main_buffer_, sub_main_buffer, dummy_side_buffer_);
            conflict_analyzer_.prepareBuffer();
            conflict_analyzer_.process(sub_main_buffer, dummy_side_buffer_);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::processSolo(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                            juce::AudioBuffer<FloatType> &sub_side_buffer) {
        if (c_solo_side_) {
            sub_main_buffer.makeCopyOf(sub_side_buffer, true);
        }
        solo_filter_.processPre(sub_main_buffer);
        switch (current_filter_lrs_[c_solo_idx_]) {
            case lrType::kStereo: {
                solo_filter_.process(sub_main_buffer);
                break;
            }
            case lrType::kLeft: {
                lr_main_splitter_.split(sub_main_buffer);
                solo_filter_.process(lr_main_splitter_.getLBuffer());
                lr_main_splitter_.getRBuffer().applyGain(0);
                lr_main_splitter_.combine(sub_main_buffer);
                break;
            }
            case lrType::kRight: {
                lr_main_splitter_.split(sub_main_buffer);
                solo_filter_.process(lr_main_splitter_.getRBuffer());
                lr_main_splitter_.getLBuffer().applyGain(0);
                lr_main_splitter_.combine(sub_main_buffer);
                break;
            }
            case lrType::kMid: {
                ms_main_splitter_.split(sub_main_buffer);
                solo_filter_.process(ms_main_splitter_.getMBuffer());
                ms_main_splitter_.getSBuffer().applyGain(0);
                ms_main_splitter_.combine(sub_main_buffer);
                break;
            }
            case lrType::kSide: {
                ms_main_splitter_.split(sub_main_buffer);
                solo_filter_.process(ms_main_splitter_.getSBuffer());
                ms_main_splitter_.getMBuffer().applyGain(0);
                ms_main_splitter_.combine(sub_main_buffer);
                break;
            }
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processDynamic(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                               juce::AudioBuffer<FloatType> &sub_side_buffer) {
        // set the auto threshold
        if (!IsBypassed) {
            for (size_t idx = 0; idx < dynamic_on_indices_.size(); ++idx) {
                const auto i = dynamic_on_indices_[idx];
                if (c_is_hist_on_[i]) {
                    const auto depThres =
                            thresholds_[i].load() + FloatType(40) +
                            static_cast<FloatType>(threshold::range.snapToLegalValue(
                                static_cast<float>(-sub_histograms_[i].getPercentile(FloatType(0.5)))));
                    filters_[i].getComputer().setThreshold(depThres);
                } else {
                    filters_[i].getComputer().setThreshold(thresholds_[i].load());
                }
            }
        }
        // stereo filters process
        processDynamicLRMSTrackers<0>(sub_side_buffer);
        processDynamicLRMS<IsBypassed, 0, 0>(sub_main_buffer, sub_side_buffer, sub_side_buffer);
        // LR filters process
        if (use_lr_) {
            lr_main_splitter_.split(sub_main_buffer);
            lr_side_splitter_.split(sub_side_buffer);
            processDynamicLRMSTrackers<1>(lr_side_splitter_.getLBuffer());
            processDynamicLRMSTrackers<2>(lr_side_splitter_.getRBuffer());
            processDynamicLRMS<IsBypassed, 1, 2>(lr_main_splitter_.getLBuffer(),
                                                 lr_side_splitter_.getLBuffer(), lr_side_splitter_.getRBuffer());
            processDynamicLRMS<IsBypassed, 2, 1>(lr_main_splitter_.getRBuffer(),
                                                 lr_side_splitter_.getRBuffer(), lr_side_splitter_.getLBuffer());
            lr_main_splitter_.combine(sub_main_buffer);
        }
        // MS filters process
        if (use_ms_) {
            ms_main_splitter_.split(sub_main_buffer);
            ms_side_splitter_.split(sub_side_buffer);
            processDynamicLRMSTrackers<3>(ms_side_splitter_.getMBuffer());
            processDynamicLRMSTrackers<4>(ms_side_splitter_.getSBuffer());
            processDynamicLRMS<IsBypassed, 3, 4>(ms_main_splitter_.getMBuffer(),
                                                 ms_side_splitter_.getMBuffer(), ms_side_splitter_.getSBuffer());
            processDynamicLRMS<IsBypassed, 4, 3>(ms_main_splitter_.getSBuffer(),
                                                 ms_side_splitter_.getSBuffer(), ms_side_splitter_.getMBuffer());
            ms_main_splitter_.combine(sub_main_buffer);
        }
        // set main filter gain & Q and update histograms
        if (!IsBypassed) {
            for (size_t idx = 0; idx < dynamic_on_indices_.size(); ++idx) {
                const auto i = dynamic_on_indices_[idx];
                main_ideals_[i].setGain(filters_[i].getMainFilter().template getGain<false>());
                main_ideals_[i].setQ(filters_[i].getMainFilter().template getQ<false>());
                main_IIRs_[i].setGain(filters_[i].getMainFilter().template getGain<false>());
                main_IIRs_[i].setQ(filters_[i].getMainFilter().template getQ<false>());
                if (c_is_hist_on_[i]) {
                    const auto diff = filters_[i].getBaseLine() - filters_[i].getTracker().getMomentaryLoudness();
                    if (diff <= 100) {
                        const auto histIdx = juce::jlimit(0, 79, juce::roundToInt(diff));
                        histograms_[i].push(static_cast<size_t>(histIdx));
                        sub_histograms_[i].push(static_cast<size_t>(histIdx));
                        atomic_histograms_[i].sync(histograms_[i]);
                    }
                } else if (c_is_editor_on_) {
                    side_loudness_[i].store(filters_[i].getTracker().getMomentaryLoudness() - filters_[i].getBaseLine());
                }
            }
        }
    }

    template<typename FloatType>
    template<size_t LRIdx>
    void Controller<FloatType>::processDynamicLRMSTrackers(juce::AudioBuffer<FloatType> &sub_side_buffer) {
        auto &tracker{trackers_[LRIdx]};
        if (use_trackers_[LRIdx]) {
            tracker.processBufferRMS(sub_side_buffer);
            tracker_baselines_[LRIdx] = tracker.getMomentaryLoudness();
            if (tracker_baselines_[LRIdx] <= tracker.minusInfinityDB + FloatType(1)) {
                tracker_baselines_[LRIdx] = tracker.minusInfinityDB * FloatType(0.5);
            }
        }
    }

    template<typename FloatType>
    template<bool IsBypassed, size_t LRIdx1, size_t LRIdx2>
    void Controller<FloatType>::processDynamicLRMS(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                                   juce::AudioBuffer<FloatType> &sub_side_buffer1,
                                                   juce::AudioBuffer<FloatType> &sub_side_buffer2) {
        const auto &indices{filter_lr_indices_[LRIdx1]};
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            const auto baseLine = side_swaps_[i] ? tracker_baselines_[LRIdx2] : tracker_baselines_[LRIdx1];
            if (c_dyn_relatives_[i]) {
                filters_[i].setBaseLine(baseLine);
            } else {
                filters_[i].setBaseLine(0);
            }
            juce::AudioBuffer<FloatType> &subSideBuffer = side_swaps_[i] ? sub_side_buffer2 : sub_side_buffer1;
            if (c_is_bypass_[i] || IsBypassed) {
                filters_[i].template process<true>(sub_main_buffer, subSideBuffer);
            } else {
                filters_[i].template process<false>(sub_main_buffer, subSideBuffer);
            }
        }
        if (c_is_sgc_on_ && c_filter_structure_ != filterStructure::kParallel) {
            compensation_gains_[LRIdx1].template process<IsBypassed>(sub_main_buffer);
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processParallelPost(juce::AudioBuffer<FloatType> &sub_main_buffer,
                                                    juce::AudioBuffer<FloatType> &sub_side_buffer) {
        // add parallel filters first
        processParallelPostLRMS<IsBypassed>(0, true, sub_main_buffer, sub_side_buffer);
        if (use_lr_) {
            processParallelPostLRMS<IsBypassed>(1, true, lr_main_splitter_.getLBuffer(), lr_side_splitter_.getLBuffer());
            processParallelPostLRMS<IsBypassed>(2, true, lr_main_splitter_.getRBuffer(), lr_side_splitter_.getRBuffer());
            lr_main_splitter_.combine(sub_main_buffer);
        }
        if (use_ms_) {
            processParallelPostLRMS<IsBypassed>(3, true, ms_main_splitter_.getMBuffer(), ms_side_splitter_.getMBuffer());
            processParallelPostLRMS<IsBypassed>(4, true, ms_main_splitter_.getSBuffer(), ms_side_splitter_.getSBuffer());
            ms_main_splitter_.combine(sub_main_buffer);
        }
        processParallelPostLRMS<IsBypassed>(0, false, sub_main_buffer, sub_side_buffer);
        if (c_is_sgc_on_) {
            compensation_gains_[0].template process<IsBypassed>(sub_main_buffer);
        }
        if (use_lr_) {
            lr_main_splitter_.split(sub_main_buffer);
            processParallelPostLRMS<IsBypassed>(1, false, lr_main_splitter_.getLBuffer(), lr_side_splitter_.getLBuffer());
            processParallelPostLRMS<IsBypassed>(2, false, lr_main_splitter_.getRBuffer(), lr_side_splitter_.getRBuffer());
            if (c_is_sgc_on_) {
                compensation_gains_[1].template process<IsBypassed>(lr_main_splitter_.getLBuffer());
                compensation_gains_[2].template process<IsBypassed>(lr_main_splitter_.getRBuffer());
            }
            lr_main_splitter_.combine(sub_main_buffer);
        }
        if (use_ms_) {
            ms_main_splitter_.split(sub_main_buffer);
            processParallelPostLRMS<IsBypassed>(3, false, ms_main_splitter_.getMBuffer(), ms_side_splitter_.getMBuffer());
            processParallelPostLRMS<IsBypassed>(4, false, ms_main_splitter_.getSBuffer(), ms_side_splitter_.getSBuffer());
            if (c_is_sgc_on_) {
                compensation_gains_[3].template process<IsBypassed>(ms_main_splitter_.getMBuffer());
                compensation_gains_[4].template process<IsBypassed>(ms_main_splitter_.getSBuffer());
            }
            ms_main_splitter_.combine(sub_main_buffer);
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processParallelPostLRMS(const size_t lr_idx, const bool should_parallel,
                                                        juce::AudioBuffer<FloatType> &sub_main_buffer,
                                                        juce::AudioBuffer<FloatType> &sub_side_buffer) {
        const auto &indices{filter_lr_indices_[lr_idx]};
        for (size_t idx = 0; idx < indices.size(); ++idx) {
            const auto i = indices[idx];
            if (filters_[i].getMainFilter().getShouldBeParallel() == should_parallel) {
                if (c_is_bypass_[i] || IsBypassed) {
                    filters_[i].template processParallelPost<true>(sub_main_buffer, sub_side_buffer);
                } else {
                    filters_[i].template processParallelPost<false>(sub_main_buffer, sub_side_buffer);
                }
            }
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processPrototypeCorrection(juce::AudioBuffer<FloatType> &sub_main_buffer) {
        prototype_corrections_[0].template process<IsBypassed>(sub_main_buffer);
        if (use_lr_) {
            lr_main_splitter_.split(sub_main_buffer);
            prototype_corrections_[1].template process<IsBypassed>(lr_main_splitter_.getLBuffer());
            prototype_corrections_[2].template process<IsBypassed>(lr_main_splitter_.getRBuffer());
            lr_main_splitter_.combine(sub_main_buffer);
        }
        if (use_ms_) {
            ms_main_splitter_.split(sub_main_buffer);
            prototype_corrections_[3].template process<IsBypassed>(ms_main_splitter_.getMBuffer());
            prototype_corrections_[4].template process<IsBypassed>(ms_main_splitter_.getSBuffer());
            ms_main_splitter_.combine(sub_main_buffer);
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processMixedCorrection(juce::AudioBuffer<FloatType> &sub_main_buffer) {
        mixed_corrections_[0].template process<IsBypassed>(sub_main_buffer);
        if (use_lr_) {
            lr_main_splitter_.split(sub_main_buffer);
            mixed_corrections_[1].template process<IsBypassed>(lr_main_splitter_.getLBuffer());
            mixed_corrections_[2].template process<IsBypassed>(lr_main_splitter_.getRBuffer());
            lr_main_splitter_.combine(sub_main_buffer);
        }
        if (use_ms_) {
            ms_main_splitter_.split(sub_main_buffer);
            mixed_corrections_[3].template process<IsBypassed>(ms_main_splitter_.getMBuffer());
            mixed_corrections_[4].template process<IsBypassed>(ms_main_splitter_.getSBuffer());
            ms_main_splitter_.combine(sub_main_buffer);
        }
    }

    template<typename FloatType>
    template<bool IsBypassed>
    void Controller<FloatType>::processLinear(juce::AudioBuffer<FloatType> &sub_main_buffer) {
        linear_filters_[0].template process<IsBypassed>(sub_main_buffer);
        if (c_is_sgc_on_) {
            compensation_gains_[0].template process<IsBypassed>(sub_main_buffer);
        }
        if (use_lr_) {
            lr_main_splitter_.split(sub_main_buffer);
            linear_filters_[1].template process<IsBypassed>(lr_main_splitter_.getLBuffer());
            linear_filters_[2].template process<IsBypassed>(lr_main_splitter_.getRBuffer());
            if (c_is_sgc_on_) {
                compensation_gains_[1].template process<IsBypassed>(lr_main_splitter_.getLBuffer());
                compensation_gains_[2].template process<IsBypassed>(lr_main_splitter_.getRBuffer());
            }
            lr_main_splitter_.combine(sub_main_buffer);
        }
        if (use_ms_) {
            ms_main_splitter_.split(sub_main_buffer);
            linear_filters_[3].template process<IsBypassed>(ms_main_splitter_.getMBuffer());
            linear_filters_[4].template process<IsBypassed>(ms_main_splitter_.getSBuffer());
            if (c_is_sgc_on_) {
                compensation_gains_[3].template process<IsBypassed>(ms_main_splitter_.getMBuffer());
                compensation_gains_[4].template process<IsBypassed>(ms_main_splitter_.getSBuffer());
            }
            ms_main_splitter_.combine(sub_main_buffer);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setFilterLRs(const lrType::lrTypes x, const size_t idx) {
        filers_lrs_[idx].store(x);
        to_update_lrs_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setDynamicON(const bool x, size_t idx) {
        const auto b_gain = b_filters_[idx].getGain();
        const auto b_q = b_filters_[idx].getQ();

        filters_[idx].setDynamicON(x);
        filters_[idx].getMainFilter().setGain(b_filters_[idx].getGain());
        filters_[idx].getMainFilter().setQ(b_filters_[idx].getQ());

        main_IIRs_[idx].setGain(b_gain);
        main_IIRs_[idx].setQ(b_q);
        main_ideals_[idx].setGain(b_gain);
        main_ideals_[idx].setQ(b_q);

        to_update_dynamic_on_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        int currentLatency = static_cast<int>(delay_.getDelaySamples());
        if (!is_zero_latency_.load()) {
            currentLatency += static_cast<int>(sub_buffer_.getLatencySamples());
        }
        currentLatency += latency_.load();
        processor_ref_.setLatencySamples(currentLatency);
    }

    template<typename FloatType>
    std::tuple<FloatType, FloatType> Controller<FloatType>::getSoloFilterParas(
        const zldsp::filter::FilterType fType, const FloatType freq, const FloatType q) {
        switch (fType) {
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kLowShelf: {
                auto soloFreq = static_cast<FloatType>(std::sqrt(1) * std::sqrt(freq));
                auto scale = soloFreq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighShelf: {
                auto soloFreq = static_cast<FloatType>(
                    std::sqrt(sub_buffer_.getMainSpec().sampleRate / 2) * std::sqrt(freq));
                auto scale = soloFreq / freq;
                soloFreq = static_cast<FloatType>(std::min(std::max(soloFreq, FloatType(10)), FloatType(20000)));
                auto bw = std::max(std::log2(scale) * 2, FloatType(0.01));
                auto soloQ = 1 / (2 * std::sinh(std::log(FloatType(2)) / 2 * bw));
                soloQ = std::min(std::max(soloQ, FloatType(0.025)), FloatType(25));
                return {soloFreq, soloQ};
            }
            case zldsp::filter::FilterType::kTiltShelf: {
                return {freq, FloatType(0.025)};
            }
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kBandPass:
            case zldsp::filter::FilterType::kBandShelf:
            default: {
                return {freq, q};
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setSolo(const size_t idx, const bool isSide) {
        solo_idx_.store(idx);
        solo_side_.store(isSide);

        use_solo_.store(true);
        to_update_solo_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::clearSolo(const size_t idx, const bool isSide) {
        if (idx == solo_idx_.load() && isSide == solo_side_.load()) {
            use_solo_.store(false);
            to_update_solo_.store(true);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setRelative(const size_t idx, const bool isRelative) {
        dyn_relatives_[idx].store(isRelative);
        to_update_dyn_rel_side_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setSideSwap(const size_t idx, const bool isSwap) {
        side_swaps_[idx].store(isSwap);
        to_update_dyn_rel_side_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDynamicONs() {
        dynamic_on_indices_.clear();
        for (size_t i = 0; i < kBandNUM; ++i) {
            if (filters_[i].getDynamicON()) {
                dynamic_on_indices_.push(i);
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateLRs() {
        use_lr_ = false;
        use_ms_ = false;
        for (auto &x: filter_lr_indices_) {
            x.clear();
        }
        for (size_t i = 0; i < kBandNUM; ++i) {
            if (is_active_[i].load()) {
                current_filter_lrs_[i] = filers_lrs_[i].load();
                switch (current_filter_lrs_[i]) {
                    case lrType::kStereo: {
                        filter_lr_indices_[0].push(i);
                        break;
                    }
                    case lrType::kLeft: {
                        filter_lr_indices_[1].push(i);
                        use_lr_ = true;
                        break;
                    }
                    case lrType::kRight: {
                        filter_lr_indices_[2].push(i);
                        use_lr_ = true;
                        break;
                    }
                    case lrType::kMid: {
                        filter_lr_indices_[3].push(i);
                        use_ms_ = true;
                        break;
                    }
                    case lrType::kSide: {
                        filter_lr_indices_[4].push(i);
                        use_ms_ = true;
                        break;
                    }
                }
            }
        }
        int new_latency = 0;
        switch (c_filter_structure_) {
            case filterStructure::kMinimum:
            case filterStructure::kSVF:
            case filterStructure::kParallel: {
                new_latency = 0;
                break;
            }
            case filterStructure::kMatched: {
                const auto singleLatency = prototype_corrections_[0].getLatency();
                new_latency = singleLatency
                             + static_cast<int>(use_lr_) * singleLatency
                             + static_cast<int>(use_ms_) * singleLatency;
                break;
            }
            case filterStructure::kMixed: {
                const auto single_latency = mixed_corrections_[0].getLatency();
                new_latency = single_latency
                             + static_cast<int>(use_lr_) * single_latency
                             + static_cast<int>(use_ms_) * single_latency;
                break;
            }
            case filterStructure::kLinear: {
                const auto single_latency = linear_filters_[0].getLatency();
                new_latency = single_latency
                             + static_cast<int>(use_lr_) * single_latency
                             + static_cast<int>(use_ms_) * single_latency;
                break;
            }
        }
        if (new_latency != latency_.load()) {
            const auto delay_in_seconds = static_cast<FloatType>(new_latency) / static_cast<FloatType>(sample_rate_.load());
            dummy_main_delay_.setDelaySeconds(delay_in_seconds);
            dummy_side_delay_.setDelaySeconds(delay_in_seconds);
            latency_.store(new_latency);
            triggerAsyncUpdate();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateTrackersON() {
        std::fill(use_trackers_.begin(), use_trackers_.end(), false);
        for (size_t idx = 0; idx < 5; ++idx) {
            const auto &indices{filter_lr_indices_[idx]};
            for (size_t i = 0; i < indices.size(); ++i) {
                if (c_dyn_relatives_[i]) {
                    use_trackers_[idx] = true;
                    break;
                }
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::setLearningHistON(const size_t idx, const bool isLearning) {
        is_hist_on_[idx].store(isLearning);
        to_update_hist_.store(true);
    }

    template<typename FloatType>
    void Controller<FloatType>::setLookAhead(const FloatType x) {
        delay_.setDelaySeconds(x / static_cast<FloatType>(1000));
        triggerAsyncUpdate();
    }

    template<typename FloatType>
    void Controller<FloatType>::setRMS(const FloatType x) {
        const auto rms_second = x / static_cast<FloatType>(1000);
        for (auto &f: filters_) {
            f.getTracker().setMomentarySeconds(rms_second);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateFilterStructure() {
        switch (c_filter_structure_) {
            case filterStructure::kMinimum: {
                for (auto &f: filters_) {
                    f.setFilterStructure(zldsp::filter::FilterStructure::kIIR);
                }
                break;
            }
            case filterStructure::kSVF: {
                for (auto &f: filters_) {
                    f.setFilterStructure(zldsp::filter::FilterStructure::kSVF);
                }
                break;
            }
            case filterStructure::kParallel: {
                for (auto &f: filters_) {
                    f.setFilterStructure(zldsp::filter::FilterStructure::kParallel);
                }
                break;
            }
            case filterStructure::kMatched: {
                for (auto &f: filters_) {
                    f.setFilterStructure(zldsp::filter::FilterStructure::kIIR);
                }
                for (auto &f: main_IIRs_) {
                    f.setToUpdate();
                }
                for (auto &f: main_ideals_) {
                    f.setToUpdate();
                }
                for (auto &c: prototype_corrections_) {
                    c.reset();
                }
                break;
            }
            case filterStructure::kMixed: {
                for (auto &f: filters_) {
                    f.setFilterStructure(zldsp::filter::FilterStructure::kIIR);
                }
                for (auto &f: main_IIRs_) {
                    f.setToUpdate();
                }
                for (auto &f: main_ideals_) {
                    f.setToUpdate();
                }
                for (auto &c: mixed_corrections_) {
                    c.reset();
                }
                break;
            }
            case filterStructure::kLinear: {
                for (auto &f: main_ideals_) {
                    f.setToUpdate();
                }
                for (auto &c: linear_filters_) {
                    c.reset();
                }
                for (size_t idx = 0; idx < kBandNUM; ++idx) {
                    const auto b_gain = b_filters_[idx].getGain();
                    const auto b_q = b_filters_[idx].getQ();
                    main_IIRs_[idx].setGain(b_gain);
                    main_IIRs_[idx].setQ(b_q);
                    main_ideals_[idx].setGain(b_gain);
                    main_ideals_[idx].setQ(b_q);
                }
                break;
            }
        }
        for (size_t idx = 0; idx < kBandNUM; ++idx) {
            const auto b_gain = b_filters_[idx].getGain();
            const auto b_q = b_filters_[idx].getQ();
            filters_[idx].getMainFilter().setGain(b_gain);
            filters_[idx].getMainFilter().setQ(b_q);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSgcValues() {
        for (size_t lr = 0; lr < 5; ++lr) {
            auto &indices{filter_lr_indices_[lr]};
            FloatType current_sgc{FloatType(1)};
            for (size_t idx = 0; idx < indices.size(); ++idx) {
                const auto i = indices[idx];
                if (!c_is_bypass_[i]) {
                    current_sgc *= compensations_[i].getGain();
                }
            }
            compensation_gains_[lr].setGainLinear(current_sgc);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateCorrections() {
        if (c_filter_structure_ == filterStructure::kMatched) {
            for (auto &c: prototype_corrections_) {
                c.setToUpdate();
            }
        } else if (c_filter_structure_ == filterStructure::kMixed) {
            for (auto &c: mixed_corrections_) {
                c.setToUpdate();
            }
        } else if (c_filter_structure_ == filterStructure::kLinear) {
            for (auto &c: linear_filters_) {
                c.setToUpdate();
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateSolo() {
        if (c_use_solo_) {
            c_solo_idx_ = solo_idx_.load();
            c_solo_side_ = solo_side_.load();
        } else {
            solo_filter_.setToRest();
            return;
        }

        FloatType freq, q;
        if (!c_solo_side_) {
            const auto &f{b_filters_[c_solo_idx_]};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        } else {
            const auto &f{filters_[c_solo_idx_].getSideFilter()};
            std::tie(freq, q) = getSoloFilterParas(
                f.getFilterType(), f.getFreq(), f.getQ());
        }
        solo_filter_.setFreq(freq);
        solo_filter_.setQ(q);
    }

    template<typename FloatType>
    void Controller<FloatType>::updateDynRelSide() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            if (c_dyn_relatives_[i] != dyn_relatives_[i].load() || c_side_swaps_[i] != side_swaps_[i].load()) {
                c_dyn_relatives_[i] = dyn_relatives_[i].load();
                c_side_swaps_[i] = side_swaps_[i].load();
                histograms_[i].reset(FloatType(12.5));
                sub_histograms_[i].reset(FloatType(12.5));
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::updateHistograms() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            if (c_is_hist_on_[i] != is_hist_on_[i].load()) {
                c_is_hist_on_[i] = is_hist_on_[i].load();
                histograms_[i].reset(FloatType(12.5));
                sub_histograms_[i].reset(FloatType(12.5));
            }
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
}
