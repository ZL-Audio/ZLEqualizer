// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "single_panel.hpp"
#include "sum_panel.hpp"
#include "scale_panel/scale_panel.hpp"
#include "dragger_panel/dragger_panel.hpp"
#include "solo_panel.hpp"
#include "right_click_panel.hpp"

namespace zlpanel {
    class ResponsePanel final : public juce::Component,
                                public juce::Thread,
                                private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ResponsePanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper);

        ~ResponsePanel() override;

        void paint(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBack();

        void repaintCallBackSlow();

        void updateBand();

        void updateSampleRate(double sample_rate);

        void run() override;

    private:
        static constexpr std::array kIDs{
            zlp::PFilterStatus::kID, zlp::PLRMode::kID,
            zlp::PFilterType::kID, zlp::POrder::kID, zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID,
            zlp::PDynamicON::kID,
            zlp::PTargetGain::kID,
            zlp::PSideFilterType::kID, zlp::PSideOrder::kID, zlp::PSideFreq::kID, zlp::PSideQ::kID
        };

        static constexpr size_t kNumPoints = 400;
        static constexpr double kFreqScaleConst = 20.0 * std::numbers::pi / 480000.0;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        std::array<std::atomic<float>, zlp::kBandNum> original_base_gains_{};
        std::array<std::atomic<float>, zlp::kBandNum> original_target_gains_{};
        std::atomic<float>& gain_scale_;

        std::vector<size_t> message_not_off_indices_;
        std::atomic<bool> message_to_update_panels_{};

        std::array<std::atomic<bool>, zlp::kBandNum> message_to_update_draggers_{};
        std::atomic<bool> message_to_update_draggers_total_{};
        std::atomic<bool> message_to_update_target_dragger_{};
        std::atomic<bool> message_to_update_side_dragger_{};

        SinglePanel single_panel_;
        SumPanel sum_panel_;
        ScalePanel scale_panel_;
        RightClickPanel right_click_panel_;
        DraggerPanel dragger_panel_;
        SoloPanel solo_panel_;

        float side_y_{0.f};
        std::atomic<float> width_{0.f}, height_{0.f};
        float c_width_{0.f}, c_height_{0.f};
        std::atomic<bool> to_update_bound_{};

        std::atomic<float>& eq_max_db_idx_ref_;
        float c_eq_max_db_idx_{-1.f};
        float c_k_{}, c_b_{};

        std::atomic<double> sample_rate_{0.};
        double c_sample_rate_{0.}, c_slider_max_{0.};
        double fft_max_{0.};

        std::vector<float> ws_;
        std::vector<float> xs_;

        std::array<zldsp::filter::Ideal<float, zlp::Controller::kFilterSize>, zlp::kBandNum> ideal_{};
        std::array<zldsp::filter::Ideal<float, zlp::Controller::kFilterSize / 2>, zlp::kBandNum> side_ideal_{};

        std::array<kfr::univector<float>, zlp::kBandNum> base_mags_;
        std::array<kfr::univector<float>, zlp::kBandNum> target_mags_;
        std::array<kfr::univector<float>, zlp::kBandNum> dynamic_mags_;
        std::array<kfr::univector<float>, 5> sum_mags_;

        std::array<zldsp::filter::Empty, zlp::kBandNum> empty_{};
        std::array<std::atomic<bool>, zlp::kBandNum> to_update_empty_flags_{};
        std::array<std::atomic<float>, zlp::kBandNum> target_gains_{};
        std::array<std::atomic<bool>, zlp::kBandNum> to_update_target_gain_flags_{};
        std::array<zldsp::filter::Empty, zlp::kBandNum> side_empty_{};
        std::array<std::atomic<bool>, zlp::kBandNum> to_update_side_empty_flags_{};

        std::array<std::atomic<bool>, zlp::kBandNum> dynamic_ons_{};
        std::array<bool, zlp::kBandNum> c_dynamic_ons_{};
        std::atomic<bool> to_update_dynamic_ons_{};

        std::array<std::atomic<zlp::FilterStatus>, zlp::kBandNum> filter_status_{};
        std::array<zlp::FilterStatus, zlp::kBandNum> c_filter_status_{};
        std::atomic<bool> to_update_filter_status_{};

        std::array<std::atomic<int>, zlp::kBandNum> lr_modes_{};
        std::array<int, zlp::kBandNum> c_lr_modes_{};
        std::atomic<bool> to_update_lr_modes_{};
        std::array<std::vector<size_t>, 5> on_lr_indices_{};
        std::array<bool, 5> to_update_lr_flags_{};
        std::array<bool, 5> is_lr_not_off_flags_{};

        std::array<bool, zlp::kBandNum> to_update_base_y_flags_{};
        std::array<bool, zlp::kBandNum> to_update_target_y_flags_{};
        std::array<bool, zlp::kBandNum> to_update_side_y_flags_{};

        // center x, left x, right x, center y, base button y, target button y
        std::array<std::array<std::atomic<float>, 6>, zlp::kBandNum> points_;
        // side button x, side left x, side right x
        std::array<std::array<std::atomic<float>, 3>, zlp::kBandNum> side_points_;

        std::mutex paint_mutex_;

        void parameterChanged(const juce::String& parameter_ID, float value) override;

        void updateCurveParas();

        bool updateCurveMags();

        static float getButtonMag(const zldsp::filter::FilterParameters& para);

        void updateDraggerPositions();

        void updateSoloPosition();

        void updateDrawingParas();

        void updateTargetPosition();

        void updateSidePosition();

        void updateFloatingPosition();

        std::tuple<float, float, float> getLeftCenterRightX(const zldsp::filter::FilterParameters& para) const;
    };
}
