// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType, bool isPeakMix = false>
    class RMSTracker {
    public:
        inline static FloatType minusInfinityDB = FloatType(-240);

        RMSTracker() = default;

        /**
         * call before prepare to play
         * set the maximum time length of the tracker
         * @param second the maximum time length of the tracker
         */
        void setMaximumMomentarySeconds(const FloatType second) {
            maximum_time_length_ = second;
        }

        /**
         * call before processing starts
         * @param sr sample_rate
         */
        void prepare(const double sr) {
            sample_rate_.store(sr);
            setMaximumMomentarySize(
                static_cast<size_t>(static_cast<double>(maximum_time_length_) * sr));
            reset();
            setMomentarySeconds(time_length_.load());
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (to_update_.exchange(false)) {
                c_buffer_size_ = buffer_size_.load();
                while (loudness_buffer_.size() > c_buffer_size_) {
                    m_loudness_ -= loudness_buffer_.popFront();
                }
                if (isPeakMix) {
                    c_peak_mix_ = peak_mix_.load();
                    c_peak_mix_c_ = FloatType(1) - c_peak_mix_;
                }
            }
        }

        void processSample(const FloatType x) {
            const FloatType square = isPeakMix
                                         ? std::abs(x) * (c_peak_mix_ + std::abs(x) * c_peak_mix_c_)
                                         : x * x;
            if (loudness_buffer_.size() == c_buffer_size_) {
                m_loudness_ -= loudness_buffer_.popFront();
            }
            loudness_buffer_.pushBack(square);
            m_loudness_ += square;
        }

        void processBufferRMS(juce::AudioBuffer<FloatType> &buffer) {
            FloatType ms = 0;
            for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
                auto data = buffer.getReadPointer(channel);
                ms += zldsp::vector::sumsqr(data, static_cast<size_t>(buffer.getNumSamples()));
            }
            ms = ms / static_cast<FloatType>(buffer.getNumSamples());

            if (loudness_buffer_.size() == c_buffer_size_) {
                m_loudness_ -= loudness_buffer_.popFront();
            }
            loudness_buffer_.pushBack(ms);
            m_loudness_ += ms;
        }

        /**
         * thread-safe, lock-free
         * set the time length of the tracker
         * @param second the time length of the tracker
         */
        void setMomentarySeconds(const FloatType second) {
            time_length_.store(second);
            setMomentarySize(static_cast<size_t>(static_cast<double>(second) * sample_rate_.load()));
            to_update_.store(true);
        }

        /**
         * thread-safe, lock-free
         * get the time length of the tracker
         */
        inline size_t getMomentarySize() const {
            return buffer_size_.load();
        }

        /**
         * thread-safe, lock-free
         * set the peak-mix portion
         * @param x the peak-mix portion
         */
        void setPeakMix(const FloatType x) {
            peak_mix_.store(x);
            to_update_.store(true);
        }

        FloatType getMomentaryLoudness() {
            FloatType meanSquare = m_loudness_ / static_cast<FloatType>(c_buffer_size_);
            return juce::Decibels::gainToDecibels(meanSquare, minusInfinityDB) * FloatType(0.5);
        }

    private:
        FloatType m_loudness_{0};
        zldsp::container::CircularBuffer<FloatType> loudness_buffer_{1};

        std::atomic<double> sample_rate_{48000.0};
        std::atomic<FloatType> time_length_{0};
        FloatType maximum_time_length_{0};
        size_t c_buffer_size_{1};
        std::atomic<size_t> buffer_size_{1};
        FloatType c_peak_mix_{0}, c_peak_mix_c_{1};
        std::atomic<FloatType> peak_mix_{0};
        std::atomic<bool> to_update_{true};

        void reset() {
            m_loudness_ = FloatType(0);
            loudness_buffer_.clear();
        }

        void setMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            buffer_size_.store(mSize);
        }

        void setMaximumMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            loudness_buffer_.setCapacity(mSize);
        }
    };
}
