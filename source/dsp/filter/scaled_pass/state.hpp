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
#include <array>
#include <span>
#include <vector>
#include "normalized.hpp"

namespace zldsp::filter::scaled_pass {
    class State {
    public:
        void prepare(const size_t channels) {
            states_.assign(channels * kMaxStates, 0.0);
        }

        void reset() {
            std::ranges::fill(states_, 0.0);
        }

        double process(const NormalizedCoefficients& coefficients, const size_t channel, double sample) {
            auto* states = states_.data() + channel * kMaxStates;
            auto output = coefficients.taps[0] * sample;
            size_t k{};
            for (size_t i = 0; i < coefficients.first_count; ++i) {
                const auto& rotation = coefficients.first[i];
                output += coefficients.taps[k + 1] * states[k];
                const auto next = rotation.sine * states[k] - rotation.cosine * sample;
                states[k] = rotation.sine * sample + rotation.cosine * states[k];
                sample = next;
                ++k;
            }
            for (size_t i = 0; i < coefficients.second_count; ++i) {
                const auto& b = coefficients.second[i][0];
                const auto& c = coefficients.second[i][1];
                output += coefficients.taps[k + 1] * states[k] + coefficients.taps[k + 2] * states[k + 1];
                const auto v = c.sine * sample + c.cosine * states[k];
                sample = c.sine * states[k] - c.cosine * sample;
                states[k] = b.sine * states[k + 1] - b.cosine * v;
                states[k + 1] = b.sine * v + b.cosine * states[k + 1];
                k += 2;
            }
            return output;
        }

        /**
         * process a block with fixed normalized coefficients
         * @tparam bypass whether to advance state without replacing the buffer
         * @param coefficients the normalized coefficients, fixed throughout the block
         * @param buffer the channel buffers, with no more channels than prepared
         * @param num_samples the number of samples per channel
         */
        template <bool bypass = false, typename FloatType>
        void processBlock(const NormalizedCoefficients& coefficients, std::span<FloatType*> buffer,
                          const size_t num_samples) {
            if (num_samples >= 2) {
                // avoid setup overhead for short mono blocks and first-order filters
                if (buffer.size() == 1 && num_samples >= 8 && coefficients.second_count > 0) {
                    processFixed<1, bypass>(coefficients, buffer, num_samples);
                    return;
                }
                if (buffer.size() == 2) {
                    // channel buffers must not overlap
                    processFixed<2, bypass>(coefficients, buffer, num_samples);
                    return;
                }
            }
            for (size_t i = 0; i < num_samples; ++i) {
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    const auto output = process(coefficients, channel, static_cast<double>(buffer[channel][i]));
                    if constexpr (!bypass) {
                        buffer[channel][i] = static_cast<FloatType>(output);
                    }
                }
            }
        }

    private:
        // reuse rotations and local state for two chronological samples per section
        // each sample keeps the scalar recurrence and output-tap accumulation order
        template <size_t kChannels, bool bypass, typename FloatType>
        void processFixed(const NormalizedCoefficients& coefficients, std::span<FloatType*> buffer,
                          const size_t num_samples) {
            std::array<double*, kChannels> states;
            for (size_t ch = 0; ch < kChannels; ++ch) { states[ch] = states_.data() + ch * kMaxStates; }
            size_t n = 0;
            for (; n + 1 < num_samples; n += 2) {
                std::array<std::array<double, kChannels>, 2> samples, outputs;
                for (size_t frame = 0; frame < 2; ++frame) {
                    for (size_t ch = 0; ch < kChannels; ++ch) {
                        samples[frame][ch] = static_cast<double>(buffer[ch][n + frame]);
                        outputs[frame][ch] = coefficients.taps[0] * samples[frame][ch];
                    }
                }
                size_t k = 0;
                for (size_t i = 0; i < coefficients.first_count; ++i) {
                    const auto rotation = coefficients.first[i];
                    const auto tap = coefficients.taps[k + 1];
                    std::array<double, kChannels> s;
                    for (size_t ch = 0; ch < kChannels; ++ch) { s[ch] = states[ch][k]; }
                    for (size_t frame = 0; frame < 2; ++frame) {
                        for (size_t ch = 0; ch < kChannels; ++ch) {
                            outputs[frame][ch] += tap * s[ch];
                            const auto next = rotation.sine * s[ch] - rotation.cosine * samples[frame][ch];
                            s[ch] = rotation.sine * samples[frame][ch] + rotation.cosine * s[ch];
                            samples[frame][ch] = next;
                        }
                    }
                    for (size_t ch = 0; ch < kChannels; ++ch) { states[ch][k] = s[ch]; }
                    ++k;
                }
                for (size_t i = 0; i < coefficients.second_count; ++i) {
                    const auto b = coefficients.second[i][0], c = coefficients.second[i][1];
                    const auto tap1 = coefficients.taps[k + 1], tap2 = coefficients.taps[k + 2];
                    std::array<double, kChannels> s1, s2;
                    for (size_t ch = 0; ch < kChannels; ++ch) {
                        s1[ch] = states[ch][k];
                        s2[ch] = states[ch][k + 1];
                    }
                    for (size_t frame = 0; frame < 2; ++frame) {
                        for (size_t ch = 0; ch < kChannels; ++ch) {
                            outputs[frame][ch] += tap1 * s1[ch] + tap2 * s2[ch];
                            const auto v = c.sine * samples[frame][ch] + c.cosine * s1[ch];
                            samples[frame][ch] = c.sine * s1[ch] - c.cosine * samples[frame][ch];
                            s1[ch] = b.sine * s2[ch] - b.cosine * v;
                            s2[ch] = b.sine * v + b.cosine * s2[ch];
                        }
                    }
                    for (size_t ch = 0; ch < kChannels; ++ch) {
                        states[ch][k] = s1[ch];
                        states[ch][k + 1] = s2[ch];
                    }
                    k += 2;
                }
                if constexpr (!bypass) {
                    for (size_t frame = 0; frame < 2; ++frame) {
                        for (size_t ch = 0; ch < kChannels; ++ch) {
                            buffer[ch][n + frame] = static_cast<FloatType>(outputs[frame][ch]);
                        }
                    }
                }
            }
            for (; n < num_samples; ++n) {
                for (size_t ch = 0; ch < kChannels; ++ch) {
                    const auto output = process(coefficients, ch, static_cast<double>(buffer[ch][n]));
                    if constexpr (!bypass) { buffer[ch][n] = static_cast<FloatType>(output); }
                }
            }
        }

        std::vector<double> states_{};
    };
}
