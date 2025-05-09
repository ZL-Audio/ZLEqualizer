// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <algorithm>

namespace zldsp::chore {
    enum SmoothedTypes {
        Lin, Mul, FixLin, FixMul
    };

    template<typename FloatType, SmoothedTypes SmoothedType = SmoothedTypes::Lin>
    class SmoothedValue {
    public:
        SmoothedValue() {
            setCurrentAndTarget(FloatType(0));
        }

        explicit SmoothedValue(const FloatType x) {
            setCurrentAndTarget(x);
        }

        void prepare(const double sampleRate, const double rampLengthInSeconds) {
            switch (SmoothedType) {
                case Lin:
                case Mul: {
                    max_count_ = static_cast<int>(sampleRate * rampLengthInSeconds);
                    break;
                }
                case FixLin: {
                    inc_ = static_cast<FloatType>(1.0 / (sampleRate * rampLengthInSeconds));
                    increase_inc_ = inc_;
                    decrease_inc_ = -inc_;
                    break;
                }
                case FixMul: {
                    inc_ = static_cast<FloatType>(std::pow(2.0, 1.0 / (sampleRate * rampLengthInSeconds)));
                    increase_inc_ = inc_;
                    decrease_inc_ = FloatType(1.0) / inc_;
                    break;
                }
            }
        }

        void setTarget(const FloatType x) {
            target_ = x;
            if (std::abs(current_ - target_) < FloatType(1e-10)) {
                count_ = 0;
                return;
            }
            switch (SmoothedType) {
                case Lin: {
                    inc_ = (target_ - current_) / static_cast<FloatType>(max_count_);
                    count_ = max_count_;
                    break;
                }
                case Mul: {
                    inc_ = std::exp(std::log(target_ / current_) / static_cast<FloatType>(max_count_));
                    count_ = max_count_;
                    break;
                }
                case FixLin:
                case FixMul: {
                    count_ = 1;
                    is_increasing_ = target_ > current_;
                    break;
                }
            }
        }

        void setCurrentAndTarget(const FloatType x) {
            current_ = x;
            target_ = x;
            count_ = 0;
        }

        FloatType getCurrent() const { return current_; }

        FloatType getTarget() const { return target_; }

        [[nodiscard]] bool isSmoothing() const { return count_ > 0; }

        FloatType getNext() {
            if (count_ == 0) { return current_; }
            switch (SmoothedType) {
                case Lin: {
                    current_ += inc_;
                    count_ -= 1;
                    return current_;
                }
                case Mul: {
                    current_ *= inc_;
                    count_ -= 1;
                    return current_;
                }
                case FixLin: {
                    if (is_increasing_) {
                        current_ += increase_inc_;
                        if (current_ > target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    } else {
                        current_ += decrease_inc_;
                        if (current_ < target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    }
                    return current_;
                }
                case FixMul: {
                    if (is_increasing_) {
                        current_ *= increase_inc_;
                        if (current_ > target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    } else {
                        current_ *= decrease_inc_;
                        if (current_ < target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    }
                    return current_;
                }
                default: return current_;
            }
        }

    private:
        FloatType current_{}, target_{}, inc_{};
        FloatType increase_inc_{}, decrease_inc_{};
        int max_count_{}, count_{};
        bool is_increasing_{};
    };
}
