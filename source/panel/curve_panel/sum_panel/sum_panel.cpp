// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlpanel {
    SumPanel::SumPanel(juce::AudioProcessorValueTreeState &parameters,
                       zlgui::UIBase &base,
                       zlp::Controller<double> &controller,
                       std::array<zldsp::filter::Ideal<double, 16>, 16> &base_filters,
                       std::array<zldsp::filter::Ideal<double, 16>, 16> &main_filters)
        : parameters_ref_(parameters),
          ui_base_(base), controller_ref_(controller),
          main_filters_(main_filters) {
        juce::ignoreUnused(base_filters);
        dbs_.resize(ws.size());
        for (auto &path: paths_) {
            path.preallocateSpace(static_cast<int>(zldsp::filter::kFrequencies.size() * 3));
        }
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            for (const auto &idx: kChangeIDs) {
                const auto paraID = zlp::appendSuffix(idx, i);
                parameterChanged(paraID, parameters_ref_.getRawParameterValue(paraID)->load());
                parameters_ref_.addParameterListener(paraID, this);
            }
        }
        lookAndFeelChanged();
    }

    SumPanel::~SumPanel() {
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            for (const auto &idx: kChangeIDs) {
                parameters_ref_.removeParameterListener(zlp::appendSuffix(idx, i), this);
            }
        }
    }

    void SumPanel::paint(juce::Graphics &g) {
        std::array<bool, 5> use_lrms{false, false, false, false, false};
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            const auto idx = static_cast<size_t>(controller_ref_.getFilterLRs(i));
            if (!controller_ref_.getBypass(i)) {
                use_lrms[idx] = true;
            }
        }
        if (ui_base_.getIsRenderingHardware()) {
            const auto current_thickness = curve_thickness_.load();
            for (size_t j = 0; j < use_lrms.size(); ++j) {
                if (!use_lrms[j]) { continue; }
                g.setColour(colours_[j]);
                const juce::GenericScopedTryLock lock(path_locks_[j]);
                if (lock.isLocked()) {
                    g.strokePath(recent_paths_[j], juce::PathStrokeType(
                                     current_thickness,
                                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
            }
        } else {
            for (size_t j = 0; j < use_lrms.size(); ++j) {
                if (!use_lrms[j]) { continue; }
                g.setColour(colours_[j]);
                const juce::GenericScopedTryLock lock(path_locks_[j]);
                if (lock.isLocked()) {
                    g.fillPath(recent_paths_[j]);
                }
            }
        }
    }

    bool SumPanel::checkRepaint() {
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            if (main_filters_[i].getMagOutdated()) {
                return true;
            }
        }
        if (to_repaint_.exchange(false)) {
            return true;
        }
        return false;
    }

    void SumPanel::run(const float physicalPixelScaleFactor) {
        juce::ScopedNoDenormals no_denormals;
        const auto is_hardware = ui_base_.getIsRenderingHardware();
        std::array<bool, 5> use_lrms{false, false, false, false, false};
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            const auto idx = static_cast<size_t>(lr_types_[i].load());
            if (!is_bypassed_[i].load()) {
                use_lrms[idx] = true;
            }
        }

        for (size_t j = 0; j < use_lrms.size(); ++j) {
            paths_[j].clear();
            if (!use_lrms[j]) {
                continue;
            }

            std::fill(dbs_.begin(), dbs_.end(), 0.0);
            for (size_t i = 0; i < zlstate::kBandNUM; i++) {
                auto &filter{controller_ref_.getMainIdealFilter(i)};
                if (lr_types_[i].load() == static_cast<zlp::lrType::lrTypes>(j) && !is_bypassed_[i].load()) {
                    main_filters_[i].setGain(filter.getGain());
                    main_filters_[i].setQ(filter.getQ());
                    main_filters_[i].updateMagnitude(ws);
                    main_filters_[i].addDBs(dbs_);
                }
            }

            drawCurve(paths_[j], dbs_, maximum_db_.load(), atomic_bound_.load(), false, true);
            if (!is_hardware) {
                juce::PathStrokeType stroke{
                    curve_thickness_.load(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded
                };
                stroke.createStrokedPath(stroke_paths_[j], paths_[j], {}, physicalPixelScaleFactor);
            }
        }
        if (is_hardware) {
            for (size_t j = 0; j < use_lrms.size(); ++j) {
                juce::GenericScopedLock lock(path_locks_[j]);
                recent_paths_[j] = paths_[j];
            }
        } else {
            for (size_t j = 0; j < use_lrms.size(); ++j) {
                juce::GenericScopedLock lock(path_locks_[j]);
                recent_paths_[j] = stroke_paths_[j];
            }
        }
    }

    void SumPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        const auto idx = static_cast<size_t>(parameter_id.getTrailingIntValue());
        if (parameter_id.startsWith(zlp::bypass::ID)) {
            is_bypassed_[idx].store(new_value > .5f);
        } else if (parameter_id.startsWith(zlp::lrType::ID)) {
            lr_types_[idx].store(static_cast<zlp::lrType::lrTypes>(new_value));
        }
        to_repaint_.store(true);
    }

    void SumPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store({
            bound.getX(), bound.getY() + ui_base_.getFontSize(),
            bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize()
        });
        to_repaint_.store(true);
        updateCurveThickness();
    }

    void SumPanel::lookAndFeelChanged() {
        for (size_t j = 0; j < colours_.size(); ++j) {
            colours_[j] = ui_base_.getColorMap2(j);
        }
        updateCurveThickness();
    }

    void SumPanel::updateCurveThickness() {
        curve_thickness_.store(ui_base_.getFontSize() * 0.2f * ui_base_.getSumCurveThickness());
    }
} // zlpanel
