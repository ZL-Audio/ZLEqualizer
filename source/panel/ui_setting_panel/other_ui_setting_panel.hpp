// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class OtherUISettingPanel final : public juce::Component {
    public:
        static constexpr float kHeightP = 4.f * 7.f;

        explicit OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

        void setRendererList(const juce::StringArray &rendererList);

    private:
        PluginProcessor &pRef;
        zlgui::UIBase &ui_base_;
        zlgui::NameLookAndFeel name_laf_;

        juce::Label rendering_engine_label_;
        zlgui::CompactCombobox rendering_engine_box_;
        juce::Label refresh_rate_label_;
        zlgui::CompactCombobox refresh_rate_box_;
        juce::Label fft_label_;
        zlgui::CompactLinearSlider fft_tilt_slider_, fft_speed_slider_;
        zlgui::CompactCombobox fft_order_box_;
        juce::Label curve_thick_label_;
        zlgui::CompactLinearSlider single_curve_slider_, sum_curve_slider_;
        juce::Label default_pass_filter_slope_label_;
        zlgui::CompactCombobox default_pass_filter_slope_box_;
        juce::Label dyn_link_label_;
        zlgui::CompactCombobox dyn_link_box_;
        juce::Label tooltip_label_;
        zlgui::CompactCombobox tooltip_on_box_, tooltip_lang_box_;
    };
} // zlpanel
