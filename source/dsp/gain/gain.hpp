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

#include "../chore/smoothed_value.hpp"
#include "../chore/decibels.hpp"
#include "../vector/vector.hpp"

namespace zldsp::gain {
    template<typename FloatType>
    class Gain {
    public:
        Gain() noexcept = default;

        void reset() {
            gain_.setCurrentAndTarget(FloatType(1));
        }

        void setGainLinear(FloatType new_gain) noexcept { gain_.setTarget(new_gain); }

        void setGainDecibels(FloatType new_gain_decibels) noexcept {
            setGainLinear(chore::decibelsToGain<FloatType>(new_gain_decibels));
        }

        FloatType getTargetGainLinear() const noexcept { return gain_.getTarget(); }

        FloatType getTargetGainDecibels() const noexcept {
            return chore::gainToDecibels<FloatType>(getTargetGainLinear());
        }

        FloatType getCurrentGainLinear() const noexcept { return gain_.getCurrent(); }

        FloatType getCurrentGainDecibels() const noexcept {
            return chore::gainToDecibels<FloatType>(getCurrentGainLinear());
        }

        [[nodiscard]] bool isSmoothing() const noexcept { return gain_.isSmoothing(); }

        void prepare(const double sample_rate, const size_t max_num_samples,
                     const double ramp_length_in_seconds) noexcept {
            gain_.prepare(sample_rate, ramp_length_in_seconds);
            gain_vs_.resize(max_num_samples);
        }

        template<bool bypass = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            if (!gain_.isSmoothing()) {
                if constexpr (bypass) return;
                for (size_t chan = 0; chan < buffer.size(); ++chan) {
                    zldsp::vector::multiply(buffer[chan], gain_.getCurrent(), num_samples);
                }
            } else {
                for (size_t idx = 0; idx < num_samples; ++idx) {
                    gain_vs_[idx] = gain_.getNext();
                }
                if constexpr (bypass) return;
                for (size_t chan = 0; chan < buffer.size(); ++chan) {
                    zldsp::vector::multiply(buffer[chan], gain_vs_.data(), num_samples);
                }
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, zldsp::chore::SmoothedTypes::kFixLin> gain_{FloatType(1)};
        kfr::univector<FloatType> gain_vs_;
    };
}
