// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <span>
#include <type_traits>

#include "../coeff/ivantsov_simd_coeff.hpp"

namespace zldsp::filter {
    /**
     * block processing for TDF, SVF and Parallel filters
     * @tparam kFilterSize the number of cascading filters
     * @tparam Coeff the coefficient type (IvantsovCoeff or IvantsovSVFCoeff)
     */
    template <size_t kFilterSize, class Coeff>
    class IIRProcessor {
        using Coefficients = std::array<double, 5>;
        using SIMDCoeff = IvantsovSIMDCoeff<Coeff>;
        static constexpr auto kTag = SIMDCoeff::kTag;
        static constexpr size_t kLanes = SIMDCoeff::kLanes;
        using CoeffChunk = std::array<std::array<double, kLanes>, 5>;

    public:
        /**
         * process a single second-order section with fixed coefficients for mono/stereo double audio
         * @tparam bypass whether to advance state without replacing the buffer
         * @tparam multiply whether to apply the output multiplier
         * @return true if processed, false if the caller must use the general processing loop
         */
        template <bool bypass = false, bool multiply = false, typename FloatType>
        static bool processSingle(const Coefficients& coeff, FloatType* s1s, FloatType* s2s,
                                  std::span<FloatType*> buffer, const size_t num_samples,
                                  const double multiplier = 1.0) {
            if constexpr (std::is_same_v<FloatType, double>) {
                if (buffer.size() == 1) {
                    processSingleFixed<1, bypass, multiply>(coeff, s1s, s2s, buffer, num_samples, multiplier);
                    return true;
                }
                if (buffer.size() == 2 && buffer[0] != buffer[1]) {
                    processSingleFixed<2, bypass, multiply>(coeff, s1s, s2s, buffer, num_samples, multiplier);
                    return true;
                }
            }
            return false;
        }

        /**
         * process complete SIMD chunks with dynamic gain
         * @param gains the linear square-root gain values
         * @return the number of processed samples; the caller handles the remainder
         */
        template <typename FloatType>
        static size_t processGainLinearChunks(const FilterType type, const size_t order, const size_t sections,
                                              const double* cache, std::span<Coefficients> coeffs,
                                              FloatType* s1s, FloatType* s2s, std::span<FloatType*> buffer,
                                              const FloatType* gains, const size_t num_samples) {
            // keep float processing on the scalar path to preserve intermediate rounding
            if constexpr (!std::is_same_v<FloatType, double> || kLanes == 1) {
                return 0;
            } else {
                if (num_samples < kLanes || order < 2 || order % 2 != 0 || sections == 0) { return 0; }
                switch (type) {
                case kPeak:
                    if (order == 2 && sections == 1) {
                        return processDynamicChunks<kPeak>(sections, cache, coeffs, s1s, s2s,
                                                           buffer, gains, num_samples);
                    }
                    if (order > 2) {
                        const auto mode = static_cast<int>(cache[0]);
                        if (mode == 3 && sections == order) {
                            return processDynamicChunks<kLowShelf, true>(sections, cache + 1, coeffs,
                                                                         s1s, s2s, buffer, gains, num_samples);
                        }
                        if (mode == 1 && sections == order / 2) {
                            return processDynamicChunks<kHighShelf>(sections, cache + 1, coeffs,
                                                                    s1s, s2s, buffer, gains, num_samples);
                        }
                        if (mode == 2 && sections == order / 2) {
                            return processDynamicChunks<kLowShelf>(sections, cache + 1, coeffs,
                                                                   s1s, s2s, buffer, gains, num_samples);
                        }
                    }
                    return 0;
                case kLowShelf:
                    if (sections == order / 2) {
                        return processDynamicChunks<kLowShelf>(sections, cache, coeffs,
                                                               s1s, s2s, buffer, gains, num_samples);
                    }
                    return 0;
                case kHighShelf:
                    if (sections == order / 2) {
                        return processDynamicChunks<kHighShelf>(sections, cache, coeffs,
                                                                s1s, s2s, buffer, gains, num_samples);
                    }
                    return 0;
                case kTiltShelf:
                    if (sections == order / 2) {
                        return processDynamicChunks<kTiltShelf>(sections, cache, coeffs,
                                                                s1s, s2s, buffer, gains, num_samples);
                    }
                    return 0;
                case kLowPass:
                case kHighPass:
                case kNotch:
                case kBandPass:
                case kFlatTilt:
                case kAllPass:
                case kFlatGain:
#if ZL_ENABLE_FILTER_SCALED_PASS
                case kScaledLowPass:
                case kScaledHighPass:
#endif
                default:
                    return 0;
                }
            }
        }

    private:
        // shared scalar recurrence for fixed and changing coefficients
        struct SectionState {
            double s1, s2;

            double process(const double sample, const Coefficients& taps) {
                if constexpr (std::is_same_v<Coeff, IvantsovSVFCoeff>) {
                    const auto theta_half = taps[1] * (sample - s1 - taps[2] * s2);
                    const auto output = taps[0] * theta_half + taps[3] * s2 + taps[4] * s1;
                    s1 += theta_half;
                    s2 = -s2 - taps[2] * theta_half;
                    return output;
                } else {
                    const auto output = sample * taps[2] + s1;
                    s1 = (sample * taps[3]) - (output * taps[0]) + s2;
                    s2 = (sample * taps[4]) - (output * taps[1]);
                    return output;
                }
            }
        };

        template <size_t kChannels>
        static auto loadStates(const double* s1s, const double* s2s) {
            std::array<SectionState, kChannels> states;
            for (size_t ch = 0; ch < kChannels; ++ch) {
                states[ch] = {s1s[ch * kFilterSize], s2s[ch * kFilterSize]};
            }
            return states;
        }

        template <size_t kChannels>
        static void saveStates(const std::array<SectionState, kChannels>& states, double* s1s, double* s2s) {
            for (size_t ch = 0; ch < kChannels; ++ch) {
                s1s[ch * kFilterSize] = states[ch].s1;
                s2s[ch * kFilterSize] = states[ch].s2;
            }
        }

        template <size_t kChannels, bool bypass, bool multiply>
        static void processSingleFixed(const Coefficients& coeff, double* s1s, double* s2s,
                                       std::span<double*> buffer, const size_t num_samples,
                                       const double multiplier) {
            auto states = loadStates<kChannels>(s1s, s2s);
            const auto taps = coeff;
            for (size_t i = 0; i < num_samples; ++i) {
                for (size_t ch = 0; ch < kChannels; ++ch) {
                    auto output = states[ch].process(buffer[ch][i], taps);
                    if constexpr (!bypass) {
                        if constexpr (multiply) { output *= multiplier; }
                        buffer[ch][i] = output;
                    }
                }
            }
            saveStates(states, s1s, s2s);
        }

        // retain mono/stereo state across all coefficient chunks
        template <FilterType type, size_t kChannels>
        HWY_NOINLINE static size_t processSingleDynamic(const double* cache, Coefficients& coeff,
                                                        double* s1s, double* s2s, std::span<double*> buffer,
                                                        const double* gains, const size_t num_samples) {
            const auto end = num_samples - num_samples % kLanes;
            if (end == 0) { return 0; }
            auto states = loadStates<kChannels>(s1s, s2s);
            CoeffChunk taps;
            for (size_t offset = 0; offset < end; offset += kLanes) {
                const auto vectors = SIMDCoeff::template getCoefficients<type>(
                    SIMDCoeff::template getGainTerms<type>(hn::LoadU(kTag, gains + offset)), cache);
                for (size_t tap = 0; tap < taps.size(); ++tap) {
                    hn::StoreU(vectors[tap], kTag, taps[tap].data());
                }
                for (size_t lane = 0; lane < kLanes; ++lane) {
                    const Coefficients sample_taps{taps[0][lane], taps[1][lane], taps[2][lane], taps[3][lane],
                                                   taps[4][lane]};
                    for (size_t ch = 0; ch < kChannels; ++ch) {
                        buffer[ch][offset + lane] = states[ch].process(buffer[ch][offset + lane], sample_taps);
                    }
                }
            }
            saveStates(states, s1s, s2s);
            for (size_t tap = 0; tap < taps.size(); ++tap) { coeff[tap] = taps[tap][kLanes - 1]; }
            return end;
        }

        // process one section of a coefficient chunk for all channels
        static void processSectionChunk(const CoeffChunk& taps, const size_t section, const size_t offset,
                                        std::span<double*> buffer, double* s1s, double* s2s) {
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                const auto state_index = channel * kFilterSize + section;
                SectionState state{s1s[state_index], s2s[state_index]};
                auto* audio = buffer[channel] + offset;
                for (size_t lane = 0; lane < kLanes; ++lane) {
                    const Coefficients sample_taps{taps[0][lane], taps[1][lane], taps[2][lane], taps[3][lane],
                                                   taps[4][lane]};
                    audio[lane] = state.process(audio[lane], sample_taps);
                }
                s1s[state_index] = state.s1;
                s2s[state_index] = state.s2;
            }
        }

        template <FilterType type, bool two_edges = false>
        static size_t processDynamicChunks(const size_t sections, const double* cache, std::span<Coefficients> coeffs,
                                           double* s1s, double* s2s, std::span<double*> buffer,
                                           const double* gains, const size_t num_samples) {
            if constexpr (!two_edges) {
                if (sections == 1) {
                    if (buffer.size() == 1) {
                        return processSingleDynamic<type, 1>(cache, coeffs[0], s1s, s2s, buffer, gains, num_samples);
                    }
                    if (buffer.size() == 2 && buffer[0] != buffer[1]) {
                        return processSingleDynamic<type, 2>(cache, coeffs[0], s1s, s2s, buffer, gains, num_samples);
                    }
                }
            }
            const auto end = num_samples - num_samples % kLanes;
            CoeffChunk taps;
            for (size_t offset = 0; offset < end; offset += kLanes) {
                const auto gain = hn::LoadU(kTag, gains + offset);
                const auto gain_terms = SIMDCoeff::template getGainTerms<type>(gain);
                typename SIMDCoeff::GainTerms inverse_gain_terms;
                if constexpr (two_edges) {
                    // preserve 1 / (1 / gain) rounding for the inverse edge
                    inverse_gain_terms = SIMDCoeff::template getGainTerms<type>(hn::Set(kTag, 1.0) / gain);
                }
                // select gain terms once per edge, preserving section order
                size_t section = 0;
                for (size_t edge = 0; edge < (two_edges ? 2u : 1u); ++edge) {
                    auto section_gain = gain_terms;
                    auto edge_end = sections;
                    if constexpr (two_edges) {
                        if (edge == 0) {
                            section_gain = inverse_gain_terms;
                            edge_end = sections / 2;
                        }
                    }
                    for (; section < edge_end; ++section) {
                        const auto vectors = SIMDCoeff::template getCoefficients<type>(
                            section_gain, cache + 3 * section);
                        for (size_t tap = 0; tap < taps.size(); ++tap) {
                            hn::StoreU(vectors[tap], kTag, taps[tap].data());
                        }
                        processSectionChunk(taps, section, offset, buffer, s1s, s2s);
                        if (offset + kLanes == end) {
                            // update stored coefficients for getCoeff() and subsequent static/bypassed processing
                            for (size_t tap = 0; tap < taps.size(); ++tap) {
                                coeffs[section][tap] = taps[tap][kLanes - 1];
                            }
                        }
                    }
                }
            }
            return end;
        }
    };
}
