// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzer,
                       zlgui::UIBase &base)
        : analyzer_ref_(analyzer), ui_base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    FFTPanel::~FFTPanel() {
        analyzer_ref_.setON(false);
    }

    void FFTPanel::paint(juce::Graphics &g) {
        juce::GenericScopedTryLock lock{path_lock_};
        if (!lock.isLocked()) { return; }
        if (analyzer_ref_.getPreON() && !recent_pre_path_.isEmpty()) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kPreColour));
            g.fillPath(recent_pre_path_);
        }
        if (analyzer_ref_.getPostON() && !recent_post_path_.isEmpty()) {
            g.setColour(ui_base_.getTextColor().withAlpha(0.5f));
            if (ui_base_.getIsRenderingHardware()) {
                g.strokePath(recent_post_path_, juce::PathStrokeType{
                                 curve_thickness_.load(),
                                 juce::PathStrokeType::curved,
                                 juce::PathStrokeType::rounded
                             });
            } else {
                g.fillPath(recent_post_stroke_path_);
            }

            g.setColour(ui_base_.getColourByIdx(zlgui::kPostColour));
            g.fillPath(recent_post_path_);
        }

        if (analyzer_ref_.getSideON() && !recent_side_path_.isEmpty()) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kSideColour));
            g.fillPath(recent_side_path_);
        }
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        left_corner_.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        right_corner_.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomic_bound_.store(bound);
        curve_thickness_.store(ui_base_.getFontSize() * 0.1f);
    }

    void FFTPanel::updatePaths(const float physicalPixelScaleFactor) {
        analyzer_ref_.updatePaths(pre_path_, post_path_, side_path_, atomic_bound_.load(), minimum_fft_db_.load());
        for (auto &path: {&pre_path_, &post_path_, &side_path_}) {
            if (!path->isEmpty()) {
                path->lineTo(right_corner_.load());
                path->lineTo(left_corner_.load());
                path->closeSubPath();
            }
        } {
            if (ui_base_.getIsRenderingHardware()) {
                juce::GenericScopedLock lock{path_lock_};
                recent_pre_path_ = pre_path_;
                recent_post_path_ = post_path_;
            } else {
                juce::PathStrokeType stroke{
                    curve_thickness_.load(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded
                };
                stroke.createStrokedPath(post_stroke_path_, post_path_, {}, physicalPixelScaleFactor);
                juce::GenericScopedLock lock{path_lock_};
                recent_pre_path_ = pre_path_;
                recent_post_path_ = post_path_;
                recent_post_stroke_path_ = post_stroke_path_;
            }
        }
        if (analyzer_ref_.getSideON()) {
            juce::GenericScopedLock lock{path_lock_};
            recent_side_path_ = side_path_;
        }
    }

    void FFTPanel::visibilityChanged() {
        analyzer_ref_.setON(isVisible());
    }
} // zlpanel
