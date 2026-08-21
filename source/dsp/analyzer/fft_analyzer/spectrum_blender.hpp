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
#include <cassert>
#include <cmath>
#include <limits>
#include <span>
#include <vector>

namespace zldsp::analyzer {
    class SpectrumBlender {
    public:
        struct Crossovers {
            float low_start{200.f};
            float low_end{400.f};
            float high_start{2000.f};
            float high_end{4000.f};
        };

        [[nodiscard]] static std::vector<float> createFrequencyGrid(
            const size_t low_fft_size,
            const size_t middle_fft_size,
            const size_t high_fft_size,
            const double sample_rate) {
            return createFrequencyGrid(low_fft_size, middle_fft_size, high_fft_size,
                                       sample_rate, Crossovers{});
        }

        [[nodiscard]] static std::vector<float> createFrequencyGrid(
            const size_t low_fft_size,
            const size_t middle_fft_size,
            const size_t high_fft_size,
            const double sample_rate,
            const Crossovers crossovers) {
            assert(low_fft_size >= 2);
            assert(middle_fft_size >= 2);
            assert(high_fft_size >= 2);
            assert(sample_rate > 0.0);
            validateCrossovers(crossovers);

            std::vector<float> frequencies;
            frequencies.reserve(low_fft_size / 2 + middle_fft_size / 2 + high_fft_size / 2 + 3);


            appendFrequencies(frequencies, low_fft_size, sample_rate,
                              -1.f, crossovers.low_end);
            appendFrequencies(frequencies, middle_fft_size, sample_rate,
                              crossovers.low_end, crossovers.high_end);
            appendFrequencies(frequencies, high_fft_size, sample_rate,
                              crossovers.high_end, std::numeric_limits<float>::infinity());

            assert(frequencies.size() >= 2);
            assert(std::ranges::is_sorted(frequencies));
            return frequencies;
        }

        static void blend(const std::span<float> output,
                          const std::span<const float> frequencies,
                          const std::span<const float> low,
                          const std::span<const float> middle,
                          const std::span<const float> high,
                          const double sample_rate) {
            blend(output, frequencies, low, middle, high, sample_rate, Crossovers{});
        }

        static void blend(const std::span<float> output,
                          const std::span<const float> frequencies,
                          const std::span<const float> low,
                          const std::span<const float> middle,
                          const std::span<const float> high,
                          const double sample_rate,
                          const Crossovers crossovers) {
            assert(output.size() == frequencies.size());
            assert(output.size() >= 2);
            assert(low.size() >= 2);
            assert(middle.size() >= 2);
            assert(high.size() >= 2);
            assert(sample_rate > 0.0);
            validateCrossovers(crossovers);

            for (size_t i = 0; i < output.size(); ++i) {
                const auto frequency = frequencies[i];
                if (frequency <= crossovers.low_start) {
                    output[i] = sampleAtFrequency(low, frequency, sample_rate);
                } else if (frequency < crossovers.low_end) {
                    const auto mix = (frequency - crossovers.low_start) /
                        (crossovers.low_end - crossovers.low_start);
                    output[i] = std::lerp(sampleAtFrequency(low, frequency, sample_rate),
                                          sampleAtFrequency(middle, frequency, sample_rate), mix);
                } else if (frequency <= crossovers.high_start) {
                    output[i] = sampleAtFrequency(middle, frequency, sample_rate);
                } else if (frequency < crossovers.high_end) {
                    const auto mix = (frequency - crossovers.high_start) /
                        (crossovers.high_end - crossovers.high_start);
                    output[i] = std::lerp(sampleAtFrequency(middle, frequency, sample_rate),
                                          sampleAtFrequency(high, frequency, sample_rate), mix);
                } else {
                    output[i] = sampleAtFrequency(high, frequency, sample_rate);
                }
            }
        }

    private:
        static void validateCrossovers([[maybe_unused]] const Crossovers crossovers) {
            assert(crossovers.low_start < crossovers.low_end);
            assert(crossovers.low_end <= crossovers.high_start);
            assert(crossovers.high_start < crossovers.high_end);
        }

        static void appendFrequencies(std::vector<float>& frequencies,
                                      const size_t fft_size,
                                      const double sample_rate,
                                      const float lower_exclusive,
                                      const float upper_inclusive) {
            const auto delta_frequency = sample_rate / static_cast<double>(fft_size);
            for (size_t bin = 0; bin <= fft_size / 2; ++bin) {
                const auto frequency = static_cast<float>(static_cast<double>(bin) * delta_frequency);
                if (frequency > lower_exclusive && frequency <= upper_inclusive) {
                    frequencies.push_back(frequency);
                }
            }
        }

        static float sampleAtFrequency(const std::span<const float> spectrum,
                                       const float frequency,
                                       const double sample_rate) {
            const auto fft_size = static_cast<double>((spectrum.size() - 1) * 2);
            const auto bin = std::clamp(static_cast<double>(frequency) * fft_size / sample_rate,
                                        0.0, static_cast<double>(spectrum.size() - 1));
            const auto lower = static_cast<size_t>(bin);
            if (lower + 1 >= spectrum.size()) {
                return spectrum.back();
            }
            const auto mix = static_cast<float>(bin - static_cast<double>(lower));
            return std::lerp(spectrum[lower], spectrum[lower + 1], mix);
        }
    };
}
