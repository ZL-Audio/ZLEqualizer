// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "over_sample_stage.hpp"
#include "halfband_coeffs.hpp"

namespace zldsp::oversample {
    /**
     *
     * @tparam FloatType
     * @tparam NumStage number of oversampling stages
     */
    template<typename FloatType, size_t NumStage>
    class OverSampler {
    private:
        static constexpr std::array kCoeff_128_05_100 = halfband_coeff::convert<FloatType>(
            halfband_coeff::kCoeff_128_05_100);
        static constexpr std::array kCoeff_32_22_100 = halfband_coeff::convert<FloatType>(
            halfband_coeff::kCoeff_32_22_100);

    public:
        OverSampler() {
            // ensure the latency is integer
            static_assert(NumStage >= 1);
            static_assert(NumStage <= 6);
            // init the first stage with a large filter
            stages_.emplace_back(OverSampleStage<FloatType>{
                std::span(kCoeff_128_05_100),
                std::span(kCoeff_128_05_100)
            });
            // init the remaining stages with a small filter
            for (size_t i = 1; i < NumStage; ++i) {
                stages_.emplace_back(OverSampleStage<FloatType>{
                    std::span(kCoeff_32_22_100),
                    std::span(kCoeff_32_22_100)
                });
            }
        }

        void prepare(const size_t num_channels, const size_t num_samples) {
            auto stage_num_samples = num_samples;
            for (size_t i = 0; i < NumStage; ++i) {
                stages_[i].prepare(num_channels, stage_num_samples);
                stage_num_samples = stage_num_samples << 1;
            }
        }

        /**
         * reset the internal oversampling states
         */
        void reset() {
            for (auto &stage: stages_) {
                stage.reset();
            }
        }

        [[nodiscard]] size_t getLatency() const {
            size_t total_latency{0};
            for (size_t i = 0; i < NumStage; ++i) {
                total_latency += stages_[i].getLatency() >> i;
            }
            return total_latency;
        }

        /**
         * process samples up
         * @param buffer input samples
         * @param num_samples
         */
        void upsample(std::span<FloatType *> buffer, const size_t num_samples) {
            auto stage_num_sample = num_samples;
            stages_[0].template upsample<true>(buffer, stage_num_sample);
            for (size_t i = 1; i < NumStage; ++i) {
                stage_num_sample = stage_num_sample << 1;
                stages_[i].template upsample<false>(stages_[i - 1].getOSPointer(), stage_num_sample);
            }
        }

        /**
         * process samples down
         * @param buffer output samples
         * @param num_samples
         */
        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            auto stage_num_sample = num_samples << (NumStage - 1);
            for (size_t i = NumStage - 1; i > 0; --i) {
                stages_[i].template downsample<false>(stages_[i - 1].getOSPointer(), stage_num_sample);
                stage_num_sample = stage_num_sample >> 1;
            }
            stages_[0].template downsample<true>(buffer, num_samples);
        }

        /**
         * @return the internal over-sampled buffer
         */
        std::vector<std::vector<FloatType> > &getOSBuffer() {
            return stages_.back().getOSBuffer();
        }

        /**
         * @return pointers to the internal over-sampled buffer
         */
        std::vector<FloatType *> &getOSPointer() {
            return stages_.back().getOSPointer();
        }

    private:
        std::vector<OverSampleStage<FloatType> > stages_;
    };
}
