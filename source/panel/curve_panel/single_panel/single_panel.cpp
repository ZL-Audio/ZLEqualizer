// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlpanel {
    SinglePanel::SinglePanel(const size_t band_idx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA,
                             zlgui::UIBase &base,
                             zlp::Controller<double> &controller,
                             zldsp::filter::Ideal<double, 16> &base_filter,
                             zldsp::filter::Ideal<double, 16> &target_filter,
                             zldsp::filter::Ideal<double, 16> &main_filter)
        : band_idx_(band_idx), parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base), controller_ref_(controller),
          reset_attach_(band_idx, parameters, parameters_NA),
          base_f_(base_filter), target_f_(target_filter), main_f_(main_filter) {
        curve_path_.preallocateSpace(static_cast<int>(zldsp::filter::kFrequencies.size() * 3 + 12));
        shadow_path_.preallocateSpace(static_cast<int>(zldsp::filter::kFrequencies.size() * 3 + 12));
        dyn_path_.preallocateSpace(static_cast<int>(zldsp::filter::kFrequencies.size() * 6 + 12));

        const std::string suffix = band_idx_ < 10 ? "0" + std::to_string(band_idx_) : std::to_string(band_idx_);
        juce::ignoreUnused(controller_ref_);

        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        parameterChanged(zlstate::active::ID + suffix,
                         parameters_NA_ref_.getRawParameterValue(zlstate::active::ID + suffix)->load());
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parameters_NA_ref_.addParameterListener(zlstate::active::ID + suffix, this);

        for (auto &id: kChangeIDs) {
            const auto para_id = id + suffix;
            parameterChanged(para_id, parameters_ref_.getRawParameterValue(para_id)->load());
            parameters_ref_.addParameterListener(para_id, this);
        }
        for (auto &id: kParaIDs) {
            const auto para_id = id + suffix;
            parameterChanged(para_id, parameters_ref_.getRawParameterValue(para_id)->load());
            parameters_ref_.addParameterListener(para_id, this);
        }

        setInterceptsMouseClicks(false, false);
        lookAndFeelChanged();
    }

    SinglePanel::~SinglePanel() {
        const std::string suffix = band_idx_ < 10 ? "0" + std::to_string(band_idx_) : std::to_string(band_idx_);
        for (auto &id: kChangeIDs) {
            parameters_ref_.removeParameterListener(id + suffix, this);
        }
        for (auto &id: kParaIDs) {
            parameters_ref_.removeParameterListener(id + suffix, this);
        }
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        parameters_NA_ref_.removeParameterListener(zlstate::active::ID + suffix, this);
    }

    void SinglePanel::paint(juce::Graphics &g) {
        if (avoid_repaint_.load()) return;
        // draw curve
        {
            g.setColour(colour_);
            const juce::GenericScopedTryLock lock(curve_lock_);
            if (lock.isLocked()) {
                if (ui_base_.getIsRenderingHardware()) {
                    g.strokePath(recent_curve_path_, juce::PathStrokeType(curve_thickness_.load(),
                                                                       juce::PathStrokeType::curved,
                                                                       juce::PathStrokeType::rounded));
                } else {
                    g.fillPath(recent_curve_path_);
                }
            }
        }
        // draw shadow
        if (selected_.load()) {
            g.setColour(colour_.withMultipliedAlpha(0.125f));
            const juce::GenericScopedTryLock lock(shadow_lock_);
            if (lock.isLocked()) {
                g.fillPath(recent_shadow_path_);
            }
        }
        // draw dynamic shadow
        if (dyn_on_.load()) {
            if (selected_.load()) {
                g.setColour(colour_.withMultipliedAlpha(0.25f));
            } else {
                g.setColour(colour_.withMultipliedAlpha(0.125f));
            }
            const juce::GenericScopedTryLock lock(dyn_lock_);
            if (lock.isLocked()) {
                g.fillPath(recent_dyn_path_);
            }
        }
        // draw the line between the curve and the button
        {
            const auto line_thickness = ui_base_.getFontSize() * 0.075f * ui_base_.getSingleCurveThickness();
            const auto pos1 = button_pos_.load(), pos2 = button_curve_pos_.load();
            g.setColour(colour_);
            g.drawLine(pos1.getX(), pos1.getY(), pos2.getX(), pos2.getY(), line_thickness);
        }
    }

    void SinglePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        atomic_bottom_left_.store(bound.getBottomLeft());
        atomic_bottom_right_.store(bound.getBottomRight());
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize());
        atomic_bound_.store(bound);
        to_repaint_.store(true);
        handleAsyncUpdate();
    }

    bool SinglePanel::checkRepaint() {
        if (!active_.load()) return false;
        if (base_f_.getMagOutdated() || (dyn_on_.load() && target_f_.getMagOutdated())) {
            return true;
        } else if (to_repaint_.exchange(false)) {
            return true;
        }
        return false;
    }

    void SinglePanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            const auto new_selected = static_cast<size_t>(new_value) == band_idx_;
            if (new_selected != selected_.load()) {
                selected_.store(static_cast<size_t>(new_value) == band_idx_);
                triggerAsyncUpdate();
            }
        } else {
            if (parameter_id.startsWith(zlstate::active::ID)) {
                active_.store(new_value > .5f);
            } else if (parameter_id.startsWith(zlp::dynamicON::ID)) {
                dyn_on_.store(new_value > .5f);
            } else if (parameter_id.startsWith(zlp::fType::ID)) {
                base_f_.setFilterType(static_cast<zldsp::filter::FilterType>(new_value));
                main_f_.setFilterType(static_cast<zldsp::filter::FilterType>(new_value));
                target_f_.setFilterType(static_cast<zldsp::filter::FilterType>(new_value));
            } else if (parameter_id.startsWith(zlp::slope::ID)) {
                const auto order = zlp::slope::orderArray[static_cast<size_t>(new_value)];
                base_f_.setOrder(order);
                main_f_.setOrder(order);
                target_f_.setOrder(order);
            } else if (parameter_id.startsWith(zlp::freq::ID)) {
                base_f_.setFreq(static_cast<double>(new_value));
                main_f_.setFreq(static_cast<double>(new_value));
                target_f_.setFreq(static_cast<double>(new_value));
            } else if (parameter_id.startsWith(zlp::gain::ID)) {
                current_base_gain_.store(static_cast<double>(new_value));
                base_f_.setGain(static_cast<double>(zlp::gain::range.snapToLegalValue(
                    new_value * static_cast<float>(scale_.load()))));
            } else if (parameter_id.startsWith(zlp::Q::ID)) {
                base_f_.setQ(static_cast<double>(new_value));
            } else if (parameter_id.startsWith(zlp::targetGain::ID)) {
                current_target_gain_.store(static_cast<double>(new_value));
                target_f_.setGain(static_cast<double>(zlp::targetGain::range.snapToLegalValue(
                    new_value * static_cast<float>(scale_.load()))));
            } else if (parameter_id.startsWith(zlp::targetQ::ID)) {
                target_f_.setQ(static_cast<double>(new_value));
            }
        }
        to_repaint_.store(true);
    }

    void SinglePanel::run(const float physicalPixelScaleFactor) {
        juce::ScopedNoDenormals no_denormals;
        const auto bound = atomic_bound_.load();
        const auto bottom_left = atomic_bottom_left_.load();
        const auto bottom_right = atomic_bottom_right_.load();
        const auto maximum_db = maximum_db_.load();
        float centered_db{0.f};
        // draw curve
        const auto baseFreq = static_cast<double>(base_f_.getFreq()); {
            if (base_f_.updateMagnitude(ws)) {
                avoid_repaint_.store(false);
            }
            curve_path_.clear();
            if (active_.load()) {
                drawCurve(curve_path_, base_f_.getDBs(), maximum_db, bound);
                centered_db = static_cast<float>(base_f_.getDB(0.0001308996938995747 * baseFreq));
            }
            if (ui_base_.getIsRenderingHardware()) {
                juce::GenericScopedLock lock(curve_lock_);
                recent_curve_path_ = curve_path_;
            } else {
                juce::PathStrokeType stroke(curve_thickness_.load(), juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded);
                stroke.createStrokedPath(stroke_path_, curve_path_, {}, physicalPixelScaleFactor);
                juce::GenericScopedLock lock(curve_lock_);
                recent_curve_path_ = stroke_path_;
            }
        }
        // draw shadow
        {
            shadow_path_.clear();
            if (active_.load()) {
                drawCurve(shadow_path_, base_f_.getDBs(), maximum_db, bound);
            }
            if (active_.load() && selected_.load()) {
                switch (base_f_.getFilterType()) {
                    case zldsp::filter::FilterType::kPeak:
                    case zldsp::filter::FilterType::kLowShelf:
                    case zldsp::filter::FilterType::kHighShelf:
                    case zldsp::filter::FilterType::kNotch:
                    case zldsp::filter::FilterType::kBandShelf:
                    case zldsp::filter::FilterType::kTiltShelf: {
                        shadow_path_.lineTo(bound.getRight(), bound.getCentreY());
                        shadow_path_.lineTo(bound.getX(), bound.getCentreY());
                        shadow_path_.closeSubPath();
                        break;
                    }
                    case zldsp::filter::FilterType::kLowPass:
                    case zldsp::filter::FilterType::kHighPass:
                    case zldsp::filter::FilterType::kBandPass: {
                        shadow_path_.lineTo(bottom_right);
                        shadow_path_.lineTo(bottom_left);
                        shadow_path_.closeSubPath();
                        break;
                    }
                }
            }
            juce::GenericScopedLock lock(shadow_lock_);
            recent_shadow_path_ = shadow_path_;
        }
        // draw dynamic shadow
        {
            dyn_path_.clear();
            if (dyn_on_.load() && active_.load()) {
                drawCurve(dyn_path_, base_f_.getDBs(), maximum_db, bound);
                target_f_.updateMagnitude(ws);
                drawCurve(dyn_path_, target_f_.getDBs(), maximum_db, bound, true, false);
                dyn_path_.closeSubPath();
            }
            juce::GenericScopedLock lock(dyn_lock_);
            recent_dyn_path_ = dyn_path_;
        }
        // update button pos
        {
            switch (base_f_.getFilterType()) {
                case zldsp::filter::FilterType::kLowShelf:
                case zldsp::filter::FilterType::kHighShelf: {
                    const auto x1 = freqToX(baseFreq, bound);
                    const auto y1 = dbToY(static_cast<float>(base_f_.getGain()) * .5f, maximum_db_.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(current_base_gain_.load()) * .5f, maximum_db_.load(), bound);
                    const auto y3 = dbToY(static_cast<float>(current_target_gain_.load()) * .5f, maximum_db_.load(), bound);
                    button_curve_pos_.store(juce::Point<float>(x1, y1));
                    button_pos_.store(juce::Point<float>(x1, y2));
                    target_button_pos_.store(juce::Point<float>(x1, y3));
                    break;
                }
                case zldsp::filter::FilterType::kPeak:
                case zldsp::filter::FilterType::kBandShelf: {
                    const auto x1 = freqToX(baseFreq, bound);
                    const auto y1 = dbToY(centered_db, maximum_db_.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(current_base_gain_.load()), maximum_db_.load(), bound);
                    const auto y3 = dbToY(static_cast<float>(current_target_gain_.load()), maximum_db_.load(), bound);
                    button_curve_pos_.store(juce::Point<float>(x1, y1));
                    button_pos_.store(juce::Point<float>(x1, y2));
                    target_button_pos_.store(juce::Point<float>(x1, y3));
                    break;
                }
                case zldsp::filter::FilterType::kTiltShelf: {
                    const auto x1 = freqToX(baseFreq, bound);
                    const auto y1 = dbToY(centered_db, maximum_db_.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(current_base_gain_.load()) * .5f, maximum_db_.load(), bound);
                    const auto y3 = dbToY(static_cast<float>(current_target_gain_.load()) * .5f, maximum_db_.load(), bound);
                    button_curve_pos_.store(juce::Point<float>(x1, y1));
                    button_pos_.store(juce::Point<float>(x1, y2));
                    target_button_pos_.store(juce::Point<float>(x1, y3));
                    break;
                }
                case zldsp::filter::FilterType::kNotch:
                case zldsp::filter::FilterType::kLowPass:
                case zldsp::filter::FilterType::kHighPass:
                case zldsp::filter::FilterType::kBandPass: {
                    const auto x1 = freqToX(baseFreq, bound);
                    const auto y1 = dbToY(centered_db, maximum_db_.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(0), maximum_db_.load(), bound);
                    button_curve_pos_.store(juce::Point<float>(x1, y1));
                    button_pos_.store(juce::Point<float>(x1, y2));
                    break;
                }
            }
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        colour_ = ui_base_.getColorMap1(band_idx_);
        handleAsyncUpdate();
    }

    void SinglePanel::handleAsyncUpdate() {
        auto thickness = selected_.load() ? ui_base_.getFontSize() * 0.15f : ui_base_.getFontSize() * 0.075f;
        thickness *= ui_base_.getSingleCurveThickness();
        curve_thickness_.store(thickness);
    }
} // zlpanel
