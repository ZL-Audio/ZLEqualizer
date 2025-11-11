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
    class MatchFFTPanel final : public juce::Component,
                                public juce::Thread {
    public:
        explicit MatchFFTPanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper);

        ~MatchFFTPanel() override;

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateSampleRate(double sample_rate);

        void run() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& fft_min_db_ref_;
        std::atomic<float>& eq_max_db_ref_;

        double sample_rate_{0.};
        AtomicBound<float> atomic_bound_{};
        float c_width_{0.f};
        std::atomic<float> min_ratio_{1.f}, max_ratio_{1.f};
        std::atomic<float> k_{1.f}, b_{0.f};

        std::vector<float> xs_{};
        std::vector<float> source_ys_{}, target_ys_{}, diff_ys_{};

        juce::Path source_path_, next_source_path_;
        juce::Path target_path_, next_target_path_;
        juce::Path diff_path_, next_diff_path_;
        std::mutex mutex_;

        void updatePath(juce::Path& path, const juce::Rectangle<float>& bound, std::span<float> ys) const;

        void visibilityChanged() override;
    };
}
