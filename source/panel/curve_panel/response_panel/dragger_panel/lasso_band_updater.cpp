// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "lasso_band_updater.hpp"

#include "../scale_panel/scale_label_panel.hpp"

namespace zlpanel {
    LassoBandUpdater::LassoBandUpdater(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        items_set_(base.getSelectedBandSet()) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto band_s = std::to_string(band);
            for (size_t i = 0; i < kScaleIDs.size(); ++i) {
                scale_paras_[i][band] = p_ref_.parameters_.getParameter(kScaleIDs[i] + band_s);
            }
            for (size_t i = 0; i < kSyncIDs.size(); ++i) {
                sync_paras_[i][band] = p_ref_.parameters_.getParameter(kSyncIDs[i] + band_s);
            }
            for (size_t i = 0; i < kShiftIDs.size(); ++i) {
                shift_paras_[i][band] = p_ref_.parameters_.getParameter(kShiftIDs[i] + band_s);
            }
        }
    }

    LassoBandUpdater::~LassoBandUpdater() {
        if (previous_band_ < zlp::kBandNum) {
            listenerAddRemove<false>(previous_band_);
        }
    }

    void LassoBandUpdater::updateBand() {
        if (previous_band_ < zlp::kBandNum) {
            listenerAddRemove<false>(previous_band_);
            for (size_t i = 0; i < kScaleIDs.size(); ++i) {
                to_update_scale_[i].store(false, std::memory_order_relaxed);
            }
            for (size_t i = 0; i < kSyncIDs.size(); ++i) {
                to_update_sync_[i].store(false, std::memory_order_relaxed);
            }
            for (size_t i = 0; i < kShiftIDs.size(); ++i) {
                to_update_shift_[i].store(false, std::memory_order_relaxed);
            }
        }
        previous_band_ = base_.getSelectedBand();
        if (previous_band_ < zlp::kBandNum) {
            listenerAddRemove<true>(previous_band_);
        }
    }

    void LassoBandUpdater::loadParas() {
        for (size_t i = 0; i < kScaleIDs.size(); ++i) {
            for (const auto& band: items_set_) {
                const auto* para = scale_paras_[i][band];
                scale_values_when_selected_[i][band] = para->convertFrom0to1(para->getValue());
            }
        }
        for (size_t i = 0; i < kShiftIDs.size(); ++i) {
            for (const auto& band: items_set_) {
                const auto* para = shift_paras_[i][band];
                shift_values_when_selected_[i][band] = para->convertFrom0to1(para->getValue());
            }
        }
    }

    void LassoBandUpdater::repaintCallBack() {
        if (base_.getSelectedBand() == zlp::kBandNum) {
            return;
        }
        if (items_set_.getNumSelected() < 2) {
            return;
        }
        if (!whole_to_update_.exchange(false, std::memory_order::acquire)) {
            return;
        }
        if (whole_to_update_scale_.exchange(false, std::memory_order::acquire)) {
            updateScaleParas();
        }
        if (whole_to_update_sync_.exchange(false, std::memory_order::acquire)) {
            updateSyncParas();
        }
        if (whole_to_update_shift_.exchange(false, std::memory_order::acquire)) {
            updateShiftParas();
        }
    }

    template <bool add>
    void LassoBandUpdater::listenerAddRemove(const size_t band) {
        const auto band_s = std::to_string(band);
        for (auto& ID : kScaleIDs) {
            if constexpr (add) {
                p_ref_.parameters_.addParameterListener(ID + band_s, this);
            } else {
                p_ref_.parameters_.removeParameterListener(ID + band_s, this);
            }
        }
        for (auto& ID : kSyncIDs) {
            if constexpr (add) {
                p_ref_.parameters_.addParameterListener(ID + band_s, this);
            } else {
                p_ref_.parameters_.removeParameterListener(ID + band_s, this);
            }
        }
        for (auto& ID : kShiftIDs) {
            if constexpr (add) {
                p_ref_.parameters_.addParameterListener(ID + band_s, this);
            } else {
                p_ref_.parameters_.removeParameterListener(ID + band_s, this);
            }
        }
    }

    void LassoBandUpdater::parameterChanged(const juce::String& parameter_ID, float) {
        for (size_t i = 0; i < kScaleIDs.size(); ++i) {
            if (parameter_ID.startsWith(kScaleIDs[i])) {
                to_update_scale_[i].store(true, std::memory_order::relaxed);
                whole_to_update_scale_.store(true, std::memory_order::release);
                whole_to_update_.store(true, std::memory_order::release);
                return;
            }
        }
        for (size_t i = 0; i < kSyncIDs.size(); ++i) {
            if (parameter_ID.startsWith(kSyncIDs[i])) {
                to_update_sync_[i].store(true, std::memory_order::relaxed);
                whole_to_update_sync_.store(true, std::memory_order::release);
                whole_to_update_.store(true, std::memory_order::release);
                return;
            }
        }
        for (size_t i = 0; i < kShiftIDs.size(); ++i) {
            if (parameter_ID.startsWith(kShiftIDs[i])) {
                to_update_shift_[i].store(true, std::memory_order::relaxed);
                whole_to_update_shift_.store(true, std::memory_order::release);
                whole_to_update_.store(true, std::memory_order::release);
                return;
            }
        }
    }

    void LassoBandUpdater::updateScaleParas() {
        const auto selected_band = base_.getSelectedBand();
        for (size_t i = 0; i < kScaleIDs.size(); ++i) {
            if (to_update_scale_[i].exchange(false, std::memory_order::acquire)) {
                const auto* target_para = scale_paras_[i][selected_band];
                const auto target = target_para->convertFrom0to1(target_para->getValue());
                const auto source = scale_values_when_selected_[i][selected_band];
                if (std::abs(source) < 1e-2f) {
                    continue;
                }
                const auto scale = target / source;
                for (const auto& band: items_set_) {
                    if (band == selected_band) {
                        continue;
                    }
                    auto* para = scale_paras_[i][band];
                    updateValue(para, para->convertTo0to1(scale_values_when_selected_[i][band] * scale));
                }
            }
        }
    }

    void LassoBandUpdater::updateSyncParas() {
        const auto selected_band = base_.getSelectedBand();
        if (to_update_sync_[0].exchange(false, std::memory_order::acquire)) {
            const auto* target_para = sync_paras_[0][selected_band];
            const auto value = target_para->getValue();
            for (const auto& band: items_set_) {
                if (band == selected_band) {
                    continue;
                }
                auto* para = sync_paras_[0][band];
                if (para->getValue() > .1f) {
                    updateValue(para, value);
                }
            }
        }
        for (size_t i = 1; i < kSyncIDs.size(); ++i) {
            if (to_update_sync_[i].exchange(false, std::memory_order::acquire)) {
                const auto* target_para = sync_paras_[i][selected_band];
                const auto value = target_para->getValue();
                for (const auto& band: items_set_) {
                    if (band == selected_band) {
                        continue;
                    }
                    auto* para = sync_paras_[i][band];
                    updateValue(para, value);
                }
            }
        }
    }

    void LassoBandUpdater::updateShiftParas() {
        const auto selected_band = base_.getSelectedBand();
        for (size_t i = 0; i < kShiftIDs.size(); ++i) {
            if (to_update_shift_[i].exchange(false, std::memory_order::acquire)) {
                const auto* target_para = shift_paras_[i][selected_band];
                const auto target = target_para->convertFrom0to1(target_para->getValue());
                const auto source = shift_values_when_selected_[i][selected_band];
                const auto shift = target - source;
                for (const auto& band: items_set_) {
                    if (band == selected_band) {
                        continue;
                    }
                    auto* para = shift_paras_[i][band];
                    updateValue(para, para->convertTo0to1(shift_values_when_selected_[i][band] + shift));
                }
            }
        }
    }
}
