// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
                    max_count = static_cast<int>(sampleRate * rampLengthInSeconds);
                    break;
                }
                case FixLin: {
                    inc = static_cast<FloatType>(1.0 / (sampleRate * rampLengthInSeconds));
                    increase_inc = inc;
                    decrease_inc = -inc;
                    break;
                }
                case FixMul: {
                    inc = static_cast<FloatType>(std::pow(2.0, 1.0 / (sampleRate * rampLengthInSeconds)));
                    increase_inc = inc;
                    decrease_inc = FloatType(1.0) / inc;
                    break;
                }
            }
        }

        void setTarget(const FloatType x) {
            target = x;
            if (std::abs(current - target) < FloatType(1e-10)) {
                count = 0;
                return;
            }
            switch (SmoothedType) {
                case Lin: {
                    inc = (target - current) / static_cast<FloatType>(max_count);
                    count = max_count;
                    break;
                }
                case Mul: {
                    inc = std::exp(std::log(target / current) / static_cast<FloatType>(max_count));
                    count = max_count;
                    break;
                }
                case FixLin:
                case FixMul: {
                    count = 1;
                    is_increasing = target > current;
                    break;
                }
            }
        }

        void setCurrentAndTarget(const FloatType x) {
            current = x;
            target = x;
            count = 0;
        }

        FloatType getCurrent() const { return current; }

        FloatType getTarget() const { return target; }

        [[nodiscard]] bool isSmoothing() const { return count > 0; }

        FloatType getNext() {
            if (count == 0) { return current; }
            switch (SmoothedType) {
                case Lin: {
                    current += inc;
                    count -= 1;
                    return current;
                }
                case Mul: {
                    current *= inc;
                    count -= 1;
                    return current;
                }
                case FixLin: {
                    if (is_increasing) {
                        current += increase_inc;
                        if (current > target) {
                            current = target;
                            count = 0;
                        }
                    } else {
                        current += decrease_inc;
                        if (current < target) {
                            current = target;
                            count = 0;
                        }
                    }
                    return current;
                }
                case FixMul: {
                    if (is_increasing) {
                        current *= increase_inc;
                        if (current > target) {
                            current = target;
                            count = 0;
                        }
                    } else {
                        current *= decrease_inc;
                        if (current < target) {
                            current = target;
                            count = 0;
                        }
                    }
                    return current;
                }
                default: return current;
            }
        }

    private:
        FloatType current{}, target{}, inc{};
        FloatType increase_inc{}, decrease_inc{};
        int max_count{}, count{};
        bool is_increasing{};
    };
}
