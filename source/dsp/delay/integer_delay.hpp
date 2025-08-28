// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <algorithm>

#include "../vector/vector.hpp"

namespace zldsp::delay {
    template<typename FloatType>
    class IntegerDelay {
    public:
        IntegerDelay() = default;

        void reset() {
            for (auto &s : states_) {
                s.resize(static_cast<size_t>(capacity_));
                std::fill(s.begin(), s.end(), FloatType(0));
            }
            head_ = 0;
            tail_ = static_cast<int>(std::round(delay_seconds_ * sample_rate_));
        }

        void prepare(const double sample_rate,
                     const size_t max_num_samples,
                     const size_t num_channels,
                     const FloatType maximum_delay_seconds) {
            sample_rate_ = sample_rate;
            delay_seconds_ = std::min(delay_seconds_, maximum_delay_seconds);
            const auto maximum_delay_samples = static_cast<double>(maximum_delay_seconds) * sample_rate;
            capacity_ = static_cast<int>(std::ceil(maximum_delay_samples)) + static_cast<int>(max_num_samples) + 1;
            states_.resize(num_channels);

            reset();
        }

        void process(std::span<FloatType *> input, size_t num_samples) {
            // write input samples to states
            const auto next_tail = (tail_ + static_cast<int>(num_samples)) % capacity_;
            if (next_tail > tail_) {
                for (size_t chan = 0; chan < input.size(); ++chan) {
                    vector::copy(states_[chan].data() + static_cast<size_t>(tail_), input[chan], num_samples);
                }
            } else {
                const auto block1_size = static_cast<size_t>(capacity_ - tail_);
                for (size_t chan = 0; chan < input.size(); ++chan) {
                    vector::copy(states_[chan].data() + static_cast<size_t>(tail_), input[chan], block1_size);
                    vector::copy(states_[chan].data(), input[chan] + block1_size, static_cast<size_t>(next_tail));
                }
            }
            tail_ = next_tail;
            // write states to input samples
            const auto next_head = (head_ + static_cast<int>(num_samples)) % capacity_;
            if (next_head > head_) {
                for (size_t chan = 0; chan < input.size(); ++chan) {
                    vector::copy(input[chan], states_[chan].data() + static_cast<size_t>(head_), num_samples);
                }
            } else {
                const auto block1_size = static_cast<size_t>(capacity_ - head_);
                for (size_t chan = 0; chan < input.size(); ++chan) {
                    vector::copy(input[chan], states_[chan].data() + static_cast<size_t>(head_), block1_size);
                    vector::copy(input[chan] + block1_size, states_[chan].data(), static_cast<size_t>(next_head));
                }
            }
            head_ = next_head;
        }

        void setDelay(const FloatType delay_seconds) {
            const auto delay_samples = static_cast<int>(std::round(delay_seconds * sample_rate_));
            const auto pre_delay_samples = static_cast<int>(std::round(delay_seconds_ * sample_rate_));
            const auto delta = delay_samples - pre_delay_samples;
            delay_seconds_ = delay_seconds;
            if (delta < 0) {
                tail_ += delta;
                if (tail_ < 0) {
                    tail_ += capacity_;
                }
            } else {
                head_ -= delta;
                if (head_ < 0) {
                    head_ += capacity_;
                }
            }
        }

        void setDelayInSamples(const int delay_samples) {
            setDelay(static_cast<FloatType>(delay_samples) / static_cast<FloatType>(sample_rate_));
        }

        [[nodiscard]] int getDelayInSamples() const {
            return static_cast<int>(std::round(delay_seconds_ * sample_rate_));
        }

    private:
        double sample_rate_{48000.0};
        FloatType delay_seconds_{0};
        int capacity_{0}, head_{0}, tail_{0};
        std::vector<std::vector<FloatType> > states_;
    };
}
