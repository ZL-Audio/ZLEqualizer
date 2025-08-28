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
    }

    template<bool IsBypassed>
    void Controller::process(std::array<double *, 2> main_pointers,
                             std::array<double *, 2> side_pointers,
                             const size_t num_samples) {
        juce::ignoreUnused(main_pointers, side_pointers, num_samples);
    }

    template void Controller::process<true>(std::array<double *, 2>, std::array<double *, 2>, size_t);
    template void Controller::process<false>(std::array<double *, 2>, std::array<double *, 2>, size_t);
}
