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
    class SumPanel final : public juce::Component {
    public:
        explicit SumPanel(PluginProcessor& p, zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateDrawingParas(int lr, bool is_same_stereo);

        void run(size_t lr, bool to_update, bool is_not_off,
                 std::span<size_t> on_indices,
                 std::span<float> xs, float k, float b,
                 std::array<kfr::univector<float>, zlp::kBandNum>& dynamic_mags);

        void runUpdate(std::array<bool, 5>& to_update_flags_);

    private:
        static constexpr size_t kNumPoints = 400;
        static constexpr float kDiffStereoAlphaMultiplier = .75f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        int skip_next_paint_{0};

        std::array<juce::Path, 5> paths_{};
        std::array<juce::Path, 5> next_paths_{};

        std::mutex mutex_;

        std::array<bool, zlp::kBandNum> is_same_stereo_{};

        float curve_thickness_{0.f};

        kfr::univector<float> temp_db_{};

        void lookAndFeelChanged() override;
    };
}
