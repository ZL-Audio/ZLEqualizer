// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace zlpanel {
    class RefreshHandler {
    public:
        explicit RefreshHandler(const double target_refresh_rate) :
            target_refresh_rate_(sanitizeTargetRefreshRate(target_refresh_rate)) {
        }

        bool tick(const double time_stamp) {
            // discard invalid time stamp
            if (!std::isfinite(time_stamp)) {
                return false;
            }
            // record the first time stamp
            if (!has_time_stamp_) {
                has_time_stamp_ = true;
                time_stamp_ = time_stamp;
                return true;
            }
            const auto diff = time_stamp - time_stamp_;
            // handle invalid/duplicate time stamp
            if (diff <= 0.0) {
                if (diff < 0.0) {
                    time_stamp_ = time_stamp;
                    resetPendingUpdate();
                    call_count_ = 0;
                }
                return false;
            }
            time_stamp_ = time_stamp;
            // do not interpret a pause or suspension as a new display refresh rate
            const auto discontinuity_threshold = std::max(
                kMinDiscontinuitySeconds,
                average_vblank_period_ > 0.0
                ? average_vblank_period_ * kDiscontinuityPeriodCount
                : 0.0);
            // handle discontinuity
            if (diff > discontinuity_threshold) {
                resetPendingUpdate();
                call_count_ = 0;
                return true;
            }
            // update the estimated vblank period
            updateVBlankPeriod(diff, time_stamp);
            // tick after a stable number of vblank calls
            ++call_count_;
            if (call_count_ >= vblank_count_per_tick_) {
                call_count_ = 0;
                return true;
            }
            return false;
        }

        [[nodiscard]] double getActualRefreshRate() const {
            // use the target rate until the first vblank period is measured
            if (average_vblank_period_ <= 0.0) {
                return target_refresh_rate_;
            }
            // calculate the rate from the averaged period and fixed call count
            return 1.0 / average_vblank_period_ / static_cast<double>(vblank_count_per_tick_);
        }

    private:
        static constexpr double kDivisorChangeConfirmationSeconds = 2.0;
        static constexpr size_t kMeasurementCallbackCount = 32;

        static constexpr double kMinDiscontinuitySeconds = 0.25;
        static constexpr double kDiscontinuityPeriodCount = 8.0;

        static constexpr double kDivisorSnapTolerance = 0.10;

        const double target_refresh_rate_;

        bool has_time_stamp_{false};
        double time_stamp_{0.0};

        double average_vblank_period_{0.0};
        double interval_sum_{0.0};
        size_t interval_count_{0};
        double pending_divisor_start_time_{0.0};

        size_t vblank_count_per_tick_{1};
        size_t pending_vblank_count_per_tick_{0};
        size_t call_count_{0};

        [[nodiscard]] static double sanitizeTargetRefreshRate(const double target_refresh_rate) {
            return std::isfinite(target_refresh_rate) && target_refresh_rate > 0.0
                ? target_refresh_rate
                : 1.0;
        }

        void resetPendingUpdate() {
            interval_sum_ = 0.0;
            interval_count_ = 0;
            pending_vblank_count_per_tick_ = 0;
            pending_divisor_start_time_ = 0.0;
        }

        void updateVBlankPeriod(const double interval, const double time_stamp) {
            const auto first_interval = average_vblank_period_ <= 0.0;
            interval_sum_ += interval;
            ++interval_count_;
            if (!first_interval && interval_count_ < kMeasurementCallbackCount) {
                return;
            }
            // average a bounded group of callbacks so old fluctuations cannot linger
            average_vblank_period_ = interval_sum_ / static_cast<double>(interval_count_);
            interval_sum_ = 0.0;
            interval_count_ = 0;

            // keep estimates near an integer on the same divider
            const auto ratio = 1.0 / average_vblank_period_ / target_refresh_rate_;
            const auto candidate = std::max<size_t>(1, static_cast<size_t>(
                                                       std::floor(ratio + kDivisorSnapTolerance)));
            if (first_interval) {
                vblank_count_per_tick_ = candidate;
            }
            if (candidate == vblank_count_per_tick_) {
                resetPendingUpdate();
                return;
            }
            // confirm the divider before changing the fixed callback count
            if (candidate != pending_vblank_count_per_tick_) {
                pending_vblank_count_per_tick_ = candidate;
                pending_divisor_start_time_ = time_stamp;
            } else if (time_stamp - pending_divisor_start_time_ >= kDivisorChangeConfirmationSeconds) {
                vblank_count_per_tick_ = candidate;
                resetPendingUpdate();
            }
        }
    };
}
