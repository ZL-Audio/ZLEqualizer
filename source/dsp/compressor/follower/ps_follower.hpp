// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COMPRESSOR_PS_FOLLOWER_HPP
#define ZL_COMPRESSOR_PS_FOLLOWER_HPP

#include <juce_dsp/juce_dsp.h>
#include <numbers>

namespace zlCompressor {
    template<typename FloatType, bool useSmooth = false, bool usePunch = false>
    class PSFollower {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @param spec
         */
        void prepare(const juce::dsp::ProcessSpec &spec) {
            expFactor = -2.0 * std::numbers::pi * 1000.0 / spec.sampleRate;
            state = FloatType(0);
            y = FloatType(0);
            toUpdate.store(true);
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (toUpdate.exchange(false)) {
                update();
            }
        }

        /**
         * process a sample
         * @param x
         * @return
         */
        FloatType processSample(const FloatType x) {
            FloatType y0;
            if (useSmooth) {
                state = std::max(x, release * state + releaseC * x);
                const auto y1 = attack * y + attackC * state;
                const auto y2 = x >= y ? attack * y + attackC * x : release * y + releaseC * x;
                y0 = smooth * y1 + smoothC * y2;
            } else {
                y0 = x >= y ? attack * y + attackC * x : release * y + releaseC * x;
            }
            if (usePunch) {
                const auto slope0 = y0 - y;
                if (punch >= FloatType(0)) {
                    if (slope0 < slope) {
                        slope = punch * slope + punchC * slope0;
                    } else {
                        slope = slope0;
                    }
                } else {
                    if (slope0 > slope && slope >= FloatType(0)) {
                        slope = punch * slope + punchC * slope0;
                    } else {
                        slope = slope0;
                    }
                }
                y += slope;
            } else {
                y = y0;
            }
            return y;
        }

        void setAttack(const FloatType millisecond) {
            attackTime.store(std::max(FloatType(0), millisecond));
            toUpdate.store(true);
        }

        void setRelease(const FloatType millisecond) {
            releaseTime.store(std::max(FloatType(0), millisecond));
            toUpdate.store(true);
        }

        void setPunch(const FloatType x) {
            punchPortion.store(std::clamp(x, FloatType(-1), FloatType(1)));
            toUpdate.store(true);
        }

        void setSmooth(const FloatType x) {
            smoothPortion.store(std::clamp(x, FloatType(0), FloatType(1)));
            toUpdate.store(true);
        }

    private:
        FloatType y{}, state{}, slope{};
        FloatType attack{}, attackC{}, release{}, releaseC{};
        FloatType punch{}, punchC{}, smooth{}, smoothC{};
        double expFactor{-0.1308996938995747};
        std::atomic<FloatType> attackTime{1}, releaseTime{1}, smoothPortion{0}, punchPortion{0};
        std::atomic<bool> toUpdate{true};

        void update() {
            // cache atomic values
            const auto currentAttackTime = static_cast<double>(attackTime.load());
            const auto currentReleaseTime = static_cast<double>(releaseTime.load());
            const auto currentSmoothPortion = static_cast<double>(smoothPortion.load());
            const auto currentPunchPortion = static_cast<double>(punchPortion.load());
            // update attack
            if (currentAttackTime < 0.001) {
                attack = FloatType(0);
            } else {
                if (usePunch) {
                    attack = static_cast<FloatType>(std::exp(
                        expFactor / currentAttackTime / (1. - std::pow(std::abs(currentPunchPortion), 2.) * 0.125)));
                } else {
                    attack = static_cast<FloatType>(std::exp(expFactor / currentAttackTime));
                }
            }
            attackC = FloatType(1) - attack;
            // update release
            if (currentReleaseTime < 0.001) {
                release = FloatType(0);
            } else {
                release = static_cast<FloatType>(std::exp(expFactor / currentReleaseTime));
            }
            releaseC = FloatType(1) - release;
            if (useSmooth) {
                // update smooth
                smooth = static_cast<FloatType>(currentSmoothPortion);
                smoothC = FloatType(1) - smooth;
            }
            if (usePunch) {
                // update punch
                if (currentAttackTime < 0.001 || std::abs(currentPunchPortion) < 0.001) {
                    punch = FloatType(0);
                } else {
                    punch = static_cast<FloatType>(std::exp(expFactor / currentAttackTime / std::abs(currentPunchPortion)));
                }
                punchC = FloatType(1) - punch;
            }
        }
    };
}

#endif //ZL_COMPRESSOR_PS_FOLLOWER_HPP
