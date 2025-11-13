// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "juce_parameter_value.hpp"

namespace zlpanel::band_helper {
    inline size_t findOffBand(PluginProcessor& p) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (getValue(p.parameters_, zlp::PFilterStatus::kID + std::to_string(band)) < .1f) {
                return band;
            }
        }
        return zlp::kBandNum;
    }

    template <bool is_right = true>
    inline size_t findClosestBand(PluginProcessor& p, const size_t c_band) {
        if (c_band >= zlp::kBandNum) {
            return zlp::kBandNum;
        }

        const auto c_freq = getValue(p.parameters_, zlp::PFreq::kID + std::to_string(c_band));

        float closest_freq = is_right ? zlp::PFreq::kRange.end + 1.f : zlp::PFreq::kRange.start - 1.f;
        std::optional<size_t> closest_freq_index;

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            // current band, ignore
            if (band == c_band) {
                continue;
            }
            // off band, ignore
            if (getValue(p.parameters_, zlp::PFilterStatus::kID + std::to_string(band)) < .5f) {
                continue;
            }

            const auto freq = getValue(p.parameters_, zlp::PFreq::kID + std::to_string(band));
            if constexpr (is_right) {
                if (freq >= c_freq) {
                    // find closest higher freq
                    if (freq < closest_freq) {
                        closest_freq = freq;
                        closest_freq_index = band;
                    }
                }
            } else {
                if (freq <= c_freq) {
                    // find closest lower freq
                    if (freq > closest_freq) {
                        closest_freq = freq;
                        closest_freq_index = band;
                    }
                }
            }
        }

        if (closest_freq_index) {
            return closest_freq_index.value();
        } else {
            return zlp::kBandNum;
        }
    }

    inline void turnOffBand(PluginProcessor& p, const size_t band, juce::SelectedItemSet<size_t>& items_set) {
        if (items_set.isSelected(band)) {
            const auto band_array = items_set.getItemArray();
            for (const size_t& b: band_array) {
                updateValue(p.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(b)), 0.f);
            }
            items_set.deselectAll();
        } else {
            updateValue(p.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(band)), 0.f);
        }
    }

    inline void turnOnOffDynamic(PluginProcessor& p, const size_t c_band, const bool dynamic_on) {
        const auto band_s = std::to_string(c_band);

        if (dynamic_on) {
            const auto eq_max_db_idx = getValue(p.parameters_NA_, zlstate::PEQMaxDB::kID);
            const auto eq_max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(eq_max_db_idx))];
            const auto gain = getValue(p.parameters_, zlp::PGain::kID + band_s);
            auto* target_gain_para = p.parameters_.getParameter(zlp::PTargetGain::kID + band_s);
            updateValue(target_gain_para,
                        target_gain_para->convertTo0to1(
                            gain > 0.f ? gain - eq_max_db * .33f : gain + eq_max_db * .33f));
        }

        updateValue(p.parameters_.getParameter(zlp::PDynamicBypass::kID + band_s),
                    dynamic_on ? 0.f : 1.f);

        updateValue(p.parameters_.getParameter(zlp::PDynamicLearn::kID + band_s),
                    dynamic_on ? 1.f : 0.f);

        updateValue(p.parameters_.getParameter(zlp::PSideLink::kID + band_s),
                    dynamic_on ? 1.f : 0.f);

        if (!dynamic_on) {
            for (const auto& ID : {zlp::PThreshold::kID, zlp::PKneeW::kID, zlp::PAttack::kID, zlp::PRelease::kID}) {
                auto* para = p.parameters_.getParameter(ID + band_s);
                updateValue(para, para->getDefaultValue());
            }
        }
    }
}
