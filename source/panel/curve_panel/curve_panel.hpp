// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../dsp/dsp.hpp"
#include "background_panel/background_panel.hpp"
#include "fft_panel/fft_panel.hpp"
#include "sum_panel/sum_panel.hpp"
#include "sum_panel/solo_panel.hpp"
#include "single_panel/single_panel.hpp"
#include "single_panel/side_panel.hpp"
#include "button_panel/button_panel.hpp"
#include "conflict_panel/conflict_panel.hpp"
#include "match_panel/match_panel.hpp"
#include "loudness_display/loudness_display.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private juce::ValueTree::Listener,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor &processor,
                            zlgui::UIBase &base);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack(double nowT);

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;
        std::array<zldsp::filter::Ideal<double, 16>, 16> base_filters_, target_filters_, main_filters_;
        BackgroundPanel background_panel_;
        FFTPanel fft_panel_;
        ConflictPanel conflict_panel_;
        SumPanel sum_panel_;
        LoudnessDisplay loudness_display_;
        ButtonPanel button_panel_;
        SoloPanel solo_panel_;
        std::array<std::unique_ptr<SinglePanel>, zlstate::kBandNUM> single_panels_;
        std::array<std::unique_ptr<SidePanel>, zlstate::kBandNUM> side_panels_;
        juce::Component dummy_component_{};
        std::atomic<size_t> band_idx_;
        size_t previous_band_idx_{zlstate::kBandNUM + 1};
        MatchPanel match_panel_;
        double current_t_{0.0};
        bool to_notify_{false};

        std::atomic<bool> show_match_panel_{false};
        bool show_ui_setting_panel_{false};

        std::atomic<float> physical_pixel_scale_factor_{1.f};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;

        void run() override;
    };
}
