// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "../../container/container.hpp"

namespace zldsp::compressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class RMSTracker {
    public:
        inline static FloatType kMinusInfinityDB = FloatType(-240);

        RMSTracker() = default;

        /**
         * call before prepare to play
         * set the maximum time length of the tracker
         * @param second the maximum time length of the tracker
         */
        void setMaximumMomentarySeconds(const FloatType second) {
            maximum_time_length_ = second;
        }

        void reset() {
            square_sum_ = 0.0;
            square_buffer_.clear();
        }

        /**
         * call before processing starts
         * @param sr sample_rate
         */
        void prepare(const double sr) {
            sample_rate_.store(sr, std::memory_order::relaxed);
            setMaximumMomentarySize(static_cast<size_t>(static_cast<double>(maximum_time_length_) * sr));
            reset();
            setMomentarySeconds(time_length_.load(std::memory_order::relaxed));
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (to_update_.exchange(false, std::memory_order::acquire)) {
                c_buffer_size_ = buffer_size_.load(std::memory_order::relaxed);
                c_buffer_size_r = FloatType(1) / static_cast<FloatType>(c_buffer_size_);
                while (square_buffer_.size() > c_buffer_size_) {
                    square_sum_ -= static_cast<double>(square_buffer_.popFront());
                }
                if (c_buffer_size_ == 1) {
                    square_sum_ = static_cast<double>(square_buffer_.getFront());
                }
            }
        }

        void processSample(const FloatType x) {
            const FloatType square = x * x;
            if (square_buffer_.size() == c_buffer_size_) {
                square_sum_ -= static_cast<double>(square_buffer_.popFront());
            }
            square_sum_ = std::max(square_sum_, 0.0);
            square_buffer_.pushBack(square);
            square_sum_ += static_cast<double>(square);
        }

        /**
         * thread-safe, lock-free
         * set the time length of the tracker
         * @param second the time length of the tracker
         */
        void setMomentarySeconds(const FloatType second) {
            time_length_.store(second, std::memory_order::relaxed);
            setMomentarySize(static_cast<size_t>(static_cast<double>(second) * sample_rate_.load(std::memory_order::relaxed)));
            to_update_.store(true, std::memory_order::release);
        }

        FloatType getMomentarySeconds() const {
            return time_length_.load(std::memory_order::relaxed);
        }

        /**
         * thread-safe, lock-free
         * get the time length of the tracker
         */
        size_t getMomentarySize() const {
            return buffer_size_.load(std::memory_order::relaxed);
        }

        size_t getCurrentBufferSize() const { return c_buffer_size_; }

        FloatType getMomentarySquare() {
            return static_cast<FloatType>(square_sum_);
        }

        FloatType getMomentaryDB() {
            FloatType mean_square = static_cast<FloatType>(square_sum_) * c_buffer_size_r;
            return std::log10(std::max(FloatType(1e-10), mean_square)) * FloatType(10);
        }

    private:
        double square_sum_{0};
        container::CircularBuffer<FloatType> square_buffer_{1};

        std::atomic<double> sample_rate_{48000.0};
        std::atomic<FloatType> time_length_{0};
        FloatType maximum_time_length_{0};
        size_t c_buffer_size_{1};
        FloatType c_buffer_size_r{FloatType(1)};
        std::atomic<size_t> buffer_size_{1};
        std::atomic<bool> to_update_{true};

        void setMomentarySize(size_t size) {
            size = std::max(static_cast<size_t>(1), size);
            buffer_size_.store(size, std::memory_order::relaxed);
        }

        void setMaximumMomentarySize(size_t size) {
            size = std::max(static_cast<size_t>(1), size);
            square_buffer_.setCapacity(size);
        }
    };
}
