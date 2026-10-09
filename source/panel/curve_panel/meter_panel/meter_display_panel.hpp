// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/mag_analyzer/mag_receiver_base.hpp"
#include "meter_top_panel.hpp"

namespace zlpanel {
    class MeterDisplayPanel final : public juce::Component {
    public:
        explicit MeterDisplayPanel(PluginProcessor& p, zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBackSlow();

        void pull(zldsp::container::FIFORange range,
                  const std::vector<std::vector<float>>& pre_fifo,
                  const std::vector<std::vector<float>>& out_fifo);

        void run(double time_stamp);

        void reset();

    private:
        static constexpr float kMeterDecayPerSecond = 2.f;
        static constexpr double kMeterGapConvergenceSeconds = 1.0;

        struct MeterBounds {
            std::array<juce::Rectangle<float>, 2> pre{}, out{}, arrow{};
        };

        zlgui::UIBase& base_;
        MeterTopPanel meter_top_panel_;
        std::atomic<float>& fft_top_db_idx_ref_;
        std::atomic<float>& fft_min_db_idx_ref_;

        AtomicBound<float> pending_bound_;
        std::atomic<float> pending_font_size_{0.f};
        TriBuffer<MeterBounds> bounds_;

        std::array<float, 2> pre_peaks_{}, out_peaks_{};
        std::array<float, 2> previous_pre_{-240.f, -240.f};
        std::array<float, 2> pre_decay_mul_{1.f, 1.f};
        std::array<float, 2> previous_out_{-240.f, -240.f};
        std::array<float, 2> out_decay_mul_{1.f, 1.f};
        std::array<double, 2> target_gap_db_{}, gap_remaining_seconds_{};
        double previous_time_{0.0};

        std::atomic<float> out_peak_{-240.f};
        std::atomic<bool> reset_peaks_{false};

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void lookAndFeelChanged() override;
    };
}
