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

        void repaintCallBackSlow();

        void updateBand();

        void updateSampleRate(double sample_rate);

        void updateFilterType(size_t band, zldsp::filter::FilterType filter_type);

        void updateDrawingParas(size_t band, zlp::FilterStatus filter_status,
                                bool is_dynamic_on,
                                bool is_same_stereo, int lr_mode);

        zlgui::dragger::Dragger& getDragger(const size_t band) {
            return draggers_[band];
        }

        zlgui::dragger::Dragger& getTargetDragger() {
            return target_dragger_;
        }

        zlgui::dragger::Dragger& getSideDragger() {
            return side_dragger_;
        }

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

    private:
        static constexpr float kBypassAlphaMultiplier = .75f;
        static constexpr float kDiffStereoAlphaMultiplier = .5f;
        static constexpr float kDraggerSizeMultiplier = 1.4f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_{};

        std::array<zlgui::dragger::Dragger, zlp::kBandNum> draggers_;
        zlgui::dragger::Dragger target_dragger_;
        zlgui::dragger::Dragger side_dragger_;

        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, true>> dragger_freq_attachment_;
        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, false>> dragger_gain_attachment_;
        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, false>> target_dragger_attachment_;
        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, true>> side_dragger_attachment_;
        juce::NormalisableRange<float> freq_range_;
        juce::NormalisableRange<float> gain_range_;

        template <std::size_t... I>
        static std::array<zlgui::dragger::Dragger, zlp::kBandNum>
        make_dragger_array(zlgui::UIBase& base, std::index_sequence<I...>) {
            return {(static_cast<void>(I), zlgui::dragger::Dragger(base))...};
        }

        std::array<bool, zlp::kBandNum> is_dynamic_on_{};
        std::array<zldsp::filter::FilterType, zlp::kBandNum> filter_types_{};
        juce::Rectangle<float> bound_;
        float sample_rate_{0.f};
        std::atomic<float>& max_db_id_ref_;
        float c_max_db_id_{-1.f};

        void lookAndFeelChanged() override;

        void updateDraggerBound(size_t band);

        void updateDraggerAttachment(size_t band);

        void updateTargetAttachment(size_t band);

        void updateSideAttachment(size_t band);
    };
}
