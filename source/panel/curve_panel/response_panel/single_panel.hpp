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
    class SinglePanel final : public juce::Component {
    public:
        explicit SinglePanel(PluginProcessor& p, zlgui::UIBase& base,
                             std::vector<size_t>& not_off_indices);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateDrawingParas(size_t band,
                                zlp::FilterStatus filter_status,
                                bool is_dynamic_on,
                                bool is_same_stereo);

        void run(size_t band,
                 zlp::FilterStatus filter_status,
                 bool to_update_base, bool to_update_target,
                 std::span<float> xs, float k, float b,
                 kfr::univector<float>& base_mag, kfr::univector<float>& target_mag,
                 float center_x, float center_mag, float button_mag,
                 bool to_update_side,
                 float left_x, float right_x);

        void runUpdate(std::array<bool, zlp::kBandNum>& to_update_base_flags,
                       std::array<bool, zlp::kBandNum>& to_update_target_flags,
                       std::array<bool, zlp::kBandNum>& to_update_side_flags);

    private:
        static constexpr size_t kNumPoints = 400;
        static constexpr float kFillingAlpha = .125f;
        static constexpr float kDynamicFillingAlpha = .33333f;
        static constexpr float kNotSelectedAlphaMultiplier = .85f;
        static constexpr float kBypassAlphaMultiplier = .75f;
        static constexpr float kDiffStereoAlphaMultiplier = .75f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::vector<size_t>& not_off_indices_;

        int skip_next_paint_{0};

        std::atomic<float> center_y_{0.f};
        float side_y_{0.f};

        std::array<juce::Path, zlp::kBandNum> base_paths_{};
        std::array<juce::Path, zlp::kBandNum> next_base_paths_{};

        std::array<juce::Path, zlp::kBandNum> base_fills_{};
        std::array<juce::Path, zlp::kBandNum> next_base_fills_{};

        std::array<juce::Path, zlp::kBandNum> target_fills_{};
        std::array<juce::Path, zlp::kBandNum> next_target_fills_{};

        std::array<juce::Line<float>, zlp::kBandNum> button_lines_{};
        std::array<juce::Line<float>, zlp::kBandNum> next_button_lines_{};

        std::array<juce::Line<float>, zlp::kBandNum> side_lines_{};
        std::array<juce::Line<float>, zlp::kBandNum> next_side_lines_{};

        std::mutex mutex_;

        std::array<float, zlp::kBandNum> base_stroke_alpha_{};
        std::array<float, zlp::kBandNum> base_fill_alpha_{};
        std::array<float, zlp::kBandNum> target_fill_alpha_{};
        std::array<juce::Colour, zlp::kBandNum> base_stroke_colour_{};
        std::array<bool, zlp::kBandNum> is_same_stereo_{};

        float curve_thickness_{0.f};

        kfr::univector<float> temp_db_{};

        template <bool thick = false>
        void drawBand(juce::Graphics& g, size_t band) const;

        void lookAndFeelChanged() override;
    };
}
