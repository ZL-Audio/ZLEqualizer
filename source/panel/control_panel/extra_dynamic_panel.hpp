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
#include "../../gui/gui.hpp"
#include "../multilingual/tooltip_helper.hpp"

#include "control_background.hpp"

namespace zlpanel {
    class ExtraDynamicPanel final : public juce::Component,
                                    private juce::ValueTree::Listener {
    public:
        explicit ExtraDynamicPanel(PluginProcessor& p, zlgui::UIBase& base,
                                   const multilingual::TooltipHelper& tooltip_helper);

        ~ExtraDynamicPanel() override;

        int getIdealHeight() const;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

        void updateBand();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        ControlBackground control_background_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label rms_length_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> rms_length_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> rms_length_attach_;

        juce::Label rms_mix_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> rms_mix_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> rms_mix_attach_;

        juce::Label dyn_smooth_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> dyn_smooth_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> dyn_smooth_attach_;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
