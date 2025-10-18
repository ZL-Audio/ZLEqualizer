// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"
#include "../../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class DraggerPanel final : public juce::Component {
    public:
        explicit DraggerPanel(PluginProcessor& p, zlgui::UIBase& base,
                              const multilingual::TooltipHelper& tooltip_helper);

        void resized() override;

        void updateDrawingParas(size_t band,
                                zlp::FilterStatus filter_status,
                                bool is_dynamic_on,
                                bool is_same_stereo);

        void updateBand();

    private:
        static constexpr float kBypassAlphaMultiplier = .75f;
        static constexpr float kDiffStereoAlphaMultiplier = .75f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        std::array<zlgui::dragger::Dragger, zlp::kBandNum> draggers_;
        std::array<zlgui::dragger::Dragger, zlp::kBandNum> target_draggers_;
        zlgui::dragger::Dragger side_dragger_;
        std::array<bool, zlp::kBandNum> is_not_off_and_dynamic_on_{};

        template <std::size_t... I>
        static std::array<zlgui::dragger::Dragger, zlp::kBandNum>
        make_dragger_array(zlgui::UIBase& base, std::index_sequence<I...>) {
            return {(static_cast<void>(I), zlgui::dragger::Dragger(base))...};
        }

        void lookAndFeelChanged() override;
    };
}
