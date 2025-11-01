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

#include "right_click_panel.hpp"

namespace zlpanel {
    class MouseEventPanel final : public juce::Component,
                                  private juce::MultiTimer {
    public:
        explicit MouseEventPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper,
                                 RightClickPanel &right_click_panel);

        ~MouseEventPanel() override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void mouseEnter(const juce::MouseEvent& event) override;

        void mouseMove(const juce::MouseEvent& event) override;

        void mouseExit(const juce::MouseEvent& event) override;

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        void mouseDrag(const juce::MouseEvent&) override;

        void updateBand();

        void updateSampleRate(double sample_rate);

        void repaintCallbackSlow();

    private:
        static constexpr std::array kInitIDs{
            zlp::PFilterStatus::kID, zlp::PFilterType::kID, zlp::PLRMode::kID,
            zlp::POrder::kID,
            zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID,
            zlp::PDynamicON::kID
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_{};
        RightClickPanel &right_click_panel_;

        size_t previous_band_{zlp::kBandNum};

        std::atomic<float>& fft_freeze_ref_;
        bool c_fft_freeze_{false};

        float fft_max_{0.f};
        float slider_max_{0.f};

        zlgui::slider::SnappingSlider q_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> q_attachment_;

        std::atomic<float>* ftype_idx_ref_{};
        float c_ftype_idx_{-1.f};
        zlgui::slider::SnappingSlider slope_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> slope_attachment_;

        void timerCallback(int timer_ID) override;

        void turnOffFFTFreeze();

        void updateSlopeAttachment();
    };
}
