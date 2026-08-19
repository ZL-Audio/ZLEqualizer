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
                    resetPendingMode();
                    call_count_ = 0;
                }
                return false;
            }
            time_stamp_ = time_stamp;
            // do not interpret a pause or suspension as a new display refresh rate.
            const auto discontinuity_threshold = std::max(
                kMinDiscontinuitySeconds,
                committed_vblank_period_ > 0.0
                ? committed_vblank_period_ * kDiscontinuityPeriodCount
                : 0.0);
            // handle discountinuity
            if (diff > discontinuity_threshold) {
                resetPendingMode();
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
            if (committed_vblank_period_ <= 0.0) {
                return target_refresh_rate_;
            }
            // calculate the rate from the committed period and fixed call count
            return 1.0 / committed_vblank_period_ / static_cast<double>(vblank_count_per_tick_);
        }

    private:
        static constexpr double kModeChangeConfirmationSeconds = 1.0;

        static constexpr double kMinDiscontinuitySeconds = 0.25;
        static constexpr double kDiscontinuityPeriodCount = 8.0;

        static constexpr double kPeriodMatchTolerance = 0.15;
        static constexpr double kDivisorSnapTolerance = 0.05;
        static constexpr double kCommittedPeriodSmoothing = 0.05;
        static constexpr double kPendingPeriodSmoothing = 0.10;

        const double target_refresh_rate_;

        bool has_time_stamp_{false};
        double time_stamp_{0.0};

        double committed_vblank_period_{0.0};
        double pending_vblank_period_{0.0};
        double pending_mode_start_time_{0.0};

        size_t vblank_count_per_tick_{1};
        size_t call_count_{0};

        [[nodiscard]] static double sanitizeTargetRefreshRate(const double target_refresh_rate) {
            return std::isfinite(target_refresh_rate) && target_refresh_rate > 0.0
                ? target_refresh_rate
                : 1.0;
        }

        void resetPendingMode() {
            pending_vblank_period_ = 0.0;
            pending_mode_start_time_ = 0.0;
        }

        void updateVBlankPeriod(const double interval, const double time_stamp) {
            // use the first valid interval as the initial vblank period
            if (committed_vblank_period_ <= 0.0) {
                commitVBlankPeriod(interval);
                return;
            }
            // normalize intervals enlarged by missed/coalesced callbacks
            const auto multiple = std::max(1LL, std::llround(interval / committed_vblank_period_));
            const auto observed_period = interval / static_cast<double>(multiple);
            // smooth observations that belong to the committed display mode
            if (periodsMatch(observed_period, committed_vblank_period_)) {
                committed_vblank_period_ += kCommittedPeriodSmoothing * (observed_period - committed_vblank_period_);
                // a direct callback rejects any pending display mode change
                if (multiple == 1) {
                    resetPendingMode();
                } else {
                    updatePendingMode(interval, time_stamp);
                }
                return;
            }
            // track an interval that may belong to a different display mode
            updatePendingMode(interval, time_stamp);
        }

        [[nodiscard]] static bool periodsMatch(const double lhs, const double rhs) {
            return std::abs(lhs / rhs - 1.0) <= kPeriodMatchTolerance;
        }

        void updatePendingMode(const double interval, const double time_stamp) {
            // start a new display mode candidate
            if (pending_vblank_period_ <= 0.0) {
                pending_vblank_period_ = interval;
                pending_mode_start_time_ = time_stamp;
                return;
            }
            // normalize missed/coalesced callbacks within the candidate mode
            const auto multiple = std::max(1LL, std::llround(interval / pending_vblank_period_));
            const auto observed_period = interval / static_cast<double>(multiple);
            // restart confirmation when the observation does not match the candidate
            if (!periodsMatch(observed_period, pending_vblank_period_)) {
                pending_vblank_period_ = interval;
                pending_mode_start_time_ = time_stamp;
                return;
            }
            pending_vblank_period_ += kPendingPeriodSmoothing * (observed_period - pending_vblank_period_);
            // commit only after the candidate remains stable for the confirmation time
            if (time_stamp - pending_mode_start_time_ >= kModeChangeConfirmationSeconds) {
                commitVBlankPeriod(pending_vblank_period_);
            }
        }

        void commitVBlankPeriod(const double vblank_period) {
            committed_vblank_period_ = vblank_period;
            // snap estimates close to an exact target multiple
            auto ratio = 1.0 / committed_vblank_period_ / target_refresh_rate_;
            const auto nearest_integer = std::round(ratio);
            if (std::abs(ratio - nearest_integer) <= kDivisorSnapTolerance) {
                ratio = nearest_integer;
            }
            // select and lock the number of vblank calls per tick
            vblank_count_per_tick_ = std::max<size_t>(1, static_cast<size_t>(std::floor(ratio)));
            call_count_ = 0;
            resetPendingMode();
        }
    };
}
