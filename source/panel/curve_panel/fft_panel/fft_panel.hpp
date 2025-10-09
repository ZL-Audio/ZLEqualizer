// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class FFTPanel final : public juce::Component,
                           public juce::Thread {
    public:
        explicit FFTPanel(PluginProcessor& p, zlgui::UIBase& base,
                          const multilingual::TooltipHelper& tooltip_helper);

        ~FFTPanel() override;

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateSampleRate(double sample_rate);

        void run() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& pre_ref_;
        std::atomic<float>& post_ref_;
        std::atomic<float>& side_ref_;
        std::atomic<float>& fft_min_db_ref_;

        double sample_rate_{0.};
        juce::Rectangle<float> bound_441_{}, bound_480_{};
        AtomicBound<float> atomic_bound_{};
        float c_width_{0.f};
        std::atomic<float> min_ratio_{1.f}, max_ratio_{1.f};

        std::vector<float> xs_{};
        std::vector<float> pre_ys_{}, post_ys_{}, side_ys_{};
        juce::Path pre_path_, next_pre_path_;
        juce::Path post_path_, next_post_path_;
        juce::Path side_path_, next_side_path_;
        std::mutex mutex_;

        bool internalRun();

        void updatePath(juce::Path &path, const juce::Rectangle<float>& bound, std::span<float> ys) const;
    };
}
