// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mouse_event_panel.hpp"
#include "../scale_panel/scale_panel.hpp"
#include "float_pop_panel.hpp"
#include "lasso_band_updater.hpp"
#include "right_click_panel.hpp"

namespace zlpanel {
    class DraggerPanel final : public juce::Component,
                               private juce::ValueTree::Listener,
                               private juce::LassoSource<size_t>,
                               private juce::ChangeListener {
    public:
        explicit DraggerPanel(PluginProcessor& p, zlgui::UIBase& base,
                              const multilingual::TooltipHelper& tooltip_helper);

        ~DraggerPanel() override;

        void resized() override;

        void repaintCallBack();

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

        FloatPopPanel& getFloatPopPanel() {
            return float_pop_panel_;
        }

        RightClickPanel& getRightClickPanel() {
            return right_click_panel_;
        }

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void mouseEnter(const juce::MouseEvent& event) override;

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    private:
        static constexpr float kBypassAlphaMultiplier = .75f;
        static constexpr float kDiffStereoAlphaMultiplier = .5f;
        static constexpr float kDraggerSizeMultiplier = 1.4f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_{};

        RightClickPanel right_click_panel_;
        MouseEventPanel mouse_event_panel_;
        ScalePanel scale_panel_;
        LassoBandUpdater lasso_band_updater_;
        juce::SelectedItemSet<size_t>& items_set_;

        std::array<zlgui::dragger::Dragger, zlp::kBandNum> draggers_;
        zlgui::dragger::Dragger target_dragger_;
        zlgui::dragger::Dragger side_dragger_;

        size_t previous_solo_whole_idx_{2 * zlp::kBandNum};
        std::array<bool, zlp::kBandNum> dragger_y_enabled_{};

        FloatPopPanel float_pop_panel_;

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

        zlgui::slider::SnappingSlider q_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> q_attachment_;

        size_t slope_attach_band_{zlp::kBandNum};
        bool is_slope_attach_side_{false};
        std::atomic<float>* ftype_idx_ref_{};
        float c_ftype_idx_{-1.f};
        std::atomic<float>* side_ftype_idx_ref_{};
        float c_side_ftype_idx_{-1.f};
        zlgui::slider::SnappingSlider slope_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> slope_attachment_;

        juce::LassoComponent<size_t> lasso_component_;

        void lookAndFeelChanged() override;

        void updateDraggerBound(size_t band);

        void updateDraggerAttachment(size_t band);

        void updateTargetAttachment(size_t band);

        void updateSideAttachment(size_t band);

        void updateSlopeAttachment();

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

        void findLassoItemsInArea(juce::Array<size_t>& items_found, const juce::Rectangle<int>& area) override;

        juce::SelectedItemSet<size_t>& getLassoSelection() override;

        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    };
}
