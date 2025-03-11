// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlChore {
    enum SmoothedTypes {
        Lin, Mul, FixLin, FixMul
    };

    template<typename FloatType, SmoothedTypes smoothedType = SmoothedTypes::Lin>
    class SmoothedValue {
    public:
        SmoothedValue(const FloatType x) {
            setCurrentAndTarget(x);
        }

        void prepare(const double sampleRate, const double rampLengthInSeconds) {
            switch (smoothedType) {
                case Lin:
                case Mul: {
                    maxCount = static_cast<int>(sampleRate * rampLengthInSeconds);
                    break;
                }
                case FixLin: {
                    inc = static_cast<FloatType>(1.0 / (sampleRate * rampLengthInSeconds));
                    increaseInc = inc;
                    decreaseInc = -inc;
                    break;
                }
                case FixMul: {
                    inc = static_cast<FloatType>(std::pow(2.0, 1.0 / (sampleRate * rampLengthInSeconds)));
                    increaseInc = inc;
                    decreaseInc = FloatType(1.0) / inc;
                    break;
                }
            }
        }

        void setTarget(const FloatType x) {
            target = x;
            switch (smoothedType) {
                case Lin: {
                    inc = (target - current) / static_cast<FloatType>(maxCount);
                    count = maxCount;
                    break;
                }
                case Mul: {
                    inc = std::exp(std::log(target / current) / static_cast<FloatType>(maxCount));
                    count = maxCount;
                    break;
                }
                case FixLin:
                case FixMul: {
                    count = 1;
                    isIncreasing = target > current;
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

        bool isSmoothing() const { return count > 0; }

        FloatType getNext() {
            if (count == 0) { return current; }
            switch (smoothedType) {
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
                    if (isIncreasing) {
                        current += increaseInc;
                        if (current > target) {
                            current = target;
                            count = 0;
                        }
                    } else {
                        current += decreaseInc;
                        if (current < target) {
                            current = target;
                            count = 0;
                        }
                    }
                    return current;
                }
                case FixMul: {
                    if (isIncreasing) {
                        current *= increaseInc;
                        if (current > target) {
                            current = target;
                            count = 0;
                        }
                    } else {
                        current *= decreaseInc;
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
        FloatType increaseInc{}, decreaseInc{};
        int maxCount{}, count{};
        bool isIncreasing{};
    };
}
