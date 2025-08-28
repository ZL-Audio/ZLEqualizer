// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 5/30/25.
//

#pragma once

#include <atomic>
#include <span>

#include "origin_gain.hpp"

namespace zldsp::gain {
    template<typename FloatType>
    class SafeGain {
    public:
        SafeGain() noexcept = default;

        void reset() {
            setGainLinear(FloatType(1));
        }

        void setGainLinear(FloatType new_gain) noexcept {
            gain_v_.store(new_gain, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setGainDecibels(FloatType new_gain_decibels) noexcept {
            setGainLinear(chore::decibelsToGain<FloatType>(new_gain_decibels));
        }

        FloatType getGainLinear() const noexcept {
            return gain_v_.load(std::memory_order::relaxed);
        }

        FloatType getGainDecibels() const noexcept {
            return chore::gainToDecibels<FloatType>(getGainLinear());
        }

        void prepare(const double sample_rate, const size_t max_num_samples,
                     const double ramp_length_in_seconds) noexcept {
            gain_.prepare(sample_rate, max_num_samples, ramp_length_in_seconds);
        }

        template<bool IsBypassed = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            if (to_update_.exchange(false, std::memory_order::acquire)) {
                gain_.setGainLinear(gain_v_.load(std::memory_order::relaxed));
            }
            gain_.template process<IsBypassed>(buffer, num_samples);
        }

    private:
        Gain<FloatType> gain_;
        std::atomic<FloatType> gain_v_{FloatType(1)};
        std::atomic<bool> to_update_{false};
    };
}
