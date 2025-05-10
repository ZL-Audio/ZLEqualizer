// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_attach.hpp"

namespace zlp {
    template<typename FloatType>
    SoloAttach<FloatType>::SoloAttach(juce::AudioProcessor &processor,
                                      juce::AudioProcessorValueTreeState &parameters,
                                      Controller<FloatType> &controller) : processor_ref_(processor),
                                                                           parameters_ref_(parameters),
                                                                           controller_ref_(controller) {
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            main_solo_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, solo::ID + suffix);
            side_solo_updater_[i] = std::make_unique<zldsp::chore::ParaUpdater>(parameters, sideSolo::ID + suffix);
        }
        addListeners();
        initDefaultValues();
    }

    template<typename FloatType>
    SoloAttach<FloatType>::~SoloAttach() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            for (auto &ID: kIDs) {
                parameters_ref_.removeParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::addListeners() {
        for (size_t i = 0; i < kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            for (auto &ID: kIDs) {
                parameters_ref_.addParameterListener(ID + suffix, this);
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::parameterChanged(const juce::String &parameter_id, float new_value) {
        const auto idx = static_cast<size_t>(parameter_id.getTrailingIntValue());
        if (parameter_id.startsWith(solo::ID) || parameter_id.startsWith(sideSolo::ID)) {
            const auto isSide = parameter_id.startsWith(sideSolo::ID);
            if (new_value > .5f) {
                if (idx != solo_idx_.load() || isSide != solo_is_side_.load()) {
                    const auto oldIdx = solo_idx_.load();
                    if (solo_is_side_.load()) {
                        side_solo_updater_[oldIdx]->update(0.f);
                    } else {
                        main_solo_updater_[oldIdx]->update(0.f);
                    }
                    solo_idx_.store(idx);
                    solo_is_side_.store(isSide);
                }
                controller_ref_.setSolo(idx, isSide);
            } else {
                controller_ref_.clearSolo(idx, isSide);
            }
        } else {
            if (controller_ref_.getSolo() && idx == solo_idx_.load()) {
                controller_ref_.setSolo(solo_idx_.load(), solo_is_side_.load());
            }
        }
    }

    template<typename FloatType>
    void SoloAttach<FloatType>::initDefaultValues() {
        for (int i = 0; i < kBandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            for (size_t j = 0; j < kDefaultVs.size(); ++j) {
                parameterChanged(kInitIDs[j] + suffix, kDefaultVs[j]);
            }
        }
    }

    template
    class SoloAttach<float>;

    template
    class SoloAttach<double>;
}
