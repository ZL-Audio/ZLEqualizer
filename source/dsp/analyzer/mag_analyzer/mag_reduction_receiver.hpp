// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include "mag_receiver.hpp"

namespace zldsp::analyzer {
    class MagReductionReceiver {
    public:
        explicit MagReductionReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 const std::vector<std::vector<float>>& pre_fifo,
                 const std::vector<std::vector<float>>& post_fifo) {
            assert(pre_fifo.size() == post_fifo.size());
            reductions_.resize(pre_fifo.size());

            for (size_t chan = 0; chan < pre_fifo.size(); ++chan) {
                const float pre_ms  = MagAnalyzerOps::calculateMS(range, pre_fifo[chan]);
                const float post_ms = MagAnalyzerOps::calculateMS(range, post_fifo[chan]);
                reductions_[chan] = chore::squareGainToDecibels(post_ms) - chore::squareGainToDecibels(pre_ms);
            }
        }

        static float calculateReduction(const zldsp::container::FIFORange range,
                                        const std::vector<std::vector<float>>& pre_fifo,
                                        const std::vector<std::vector<float>>& post_fifo,
                                        const StereoType stereo_type) {
            const float pre_db = MagReceiver::calculate(range, pre_fifo, MagType::kRMS, stereo_type);
            const float post_db = MagReceiver::calculate(range, post_fifo, MagType::kRMS, stereo_type);

            return post_db - pre_db;
        }

        auto& getReductions() { return reductions_; }

    protected:
        std::vector<float> reductions_;
    };
}
