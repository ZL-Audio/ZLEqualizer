// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"

namespace zlpanel {
    class FFTPanel final : public juce::Component {
    public:
        explicit FFTPanel(zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzer,
                          zlgui::UIBase &base);

        ~FFTPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updatePaths(float physicalPixelScaleFactor);

        void visibilityChanged() override;

        void setMinimumFFTDB(const float x) {
            minimum_fft_db_.store(x);
        }

    private:
        zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzer_ref_;
        zlgui::UIBase &ui_base_;
        juce::Path pre_path_{}, post_path_{}, post_stroke_path_{}, side_path_{};
        juce::Path recent_pre_path_{}, recent_post_path_{}, recent_post_stroke_path_{}, recent_side_path_{};
        juce::SpinLock path_lock_;
        AtomicPoint<float> left_corner_, right_corner_;
        AtomicBound<float> atomic_bound_;
        std::atomic<float> curve_thickness_{0.f};
        std::atomic<bool> first_path_ = true;
        std::atomic<float> minimum_fft_db_{-72.f};
    };
} // zlpanel
