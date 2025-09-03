// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlp {
    Controller::Controller(juce::AudioProcessor &processor)
        : p_ref_(processor) {
        juce::ignoreUnused(p_ref_);
    }

    void Controller::prepare(double sample_rate, size_t max_num_samples) {
        juce::ignoreUnused(sample_rate, max_num_samples);
        empty_.setFreq(10000.0);
        empty_.setGain(-10.0);

        filter_.prepare(sample_rate, 2, max_num_samples);
        filter_.setDynamicON(false);
        filter_.setThreshold(-20.0);
        filter_.setKnee(5.0);
        filter_.setTargetGain(-10.0);

        const auto para = empty_.getParas();
        filter_.getFilter().updateParas(para);
        filter_.prepareBuffer(empty_.getGain(), 0.0);

        ideal_[0].prepare(sample_rate);
        ideal_[0].forceUpdate(para);
        tdf_[0].prepare(sample_rate, 2, max_num_samples);
        tdf_[0].forceUpdate(para);
        correction_.prepare(sample_rate, 2);
        calculator_.prepare(correction_.getNumBin());
        update_indices[0] = 0;
        on_indices[0] = 0;
        calculator_.update(tdf_, ideal_, update_indices);
        correction_.updateCorrection(calculator_.getCorrections(), on_indices);

        p_ref_.setLatencySamples(correction_.getLatency());
    }

    template<bool IsBypassed>
    void Controller::process(std::array<double *, 2> main_pointers,
                             std::array<double *, 2> side_pointers,
                             const size_t num_samples) {
        filter_.processTDF(main_pointers, side_pointers, num_samples);
        correction_.process(main_pointers, num_samples);
    }

    template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);
    template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);
}
