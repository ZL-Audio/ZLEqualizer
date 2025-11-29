// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "extra_dynamic_panel.hpp"

namespace zlpanel {
    ExtraDynamicPanel::ExtraDynamicPanel(PluginProcessor& p,
                                         zlgui::UIBase& base,
                                         const multilingual::TooltipHelper&) :
        p_ref_(p), base_(base),
        control_background_(base),
        label_laf_(base),
        rms_length_label_("", "Length"),
        rms_length_slider_("", base),
        rms_mix_label_("", "Mix"),
        rms_mix_slider_("", base),
        dyn_smooth_label_("", "Smooth"),
        dyn_smooth_slider_("", base) {
        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        label_laf_.setFontScale(1.5f);

        for (auto& l : {&rms_length_label_, &rms_mix_label_, &dyn_smooth_label_}) {
            l->setJustificationType(juce::Justification::centredRight);
            l->setLookAndFeel(&label_laf_);
            l->setInterceptsMouseClicks(false, false);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        for (auto& s : {&rms_length_slider_, &rms_mix_slider_, &dyn_smooth_slider_}) {
            s->getSlider().setSliderSnapsToMousePosition(false);
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }

        base_.getPanelValueTree().addListener(this);
    }

    ExtraDynamicPanel::~ExtraDynamicPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int ExtraDynamicPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return getButtonSize(font_size) + getPaddingSize(font_size);
    }

    int ExtraDynamicPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSliderWidth(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto padding = getPaddingSize(font_size);

        return 5 * padding + 2 * (padding / 2) + 2 * slider_width + 2 * small_slider_width;
    }

    void ExtraDynamicPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto slider_width = getSmallSliderWidth(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound.withHeight(bound.getHeight() * 2));

        bound.reduce(padding + padding / 2, 0);
        bound.removeFromTop(padding);

        const auto t_width = (bound.getWidth() - slider_width - slider_width - slider_width / 2) / 3;
        rms_length_label_.setBounds(bound.removeFromLeft(slider_width));
        rms_length_slider_.setBounds(bound.removeFromLeft(t_width));
        rms_mix_label_.setBounds(bound.removeFromLeft(slider_width / 2));
        rms_mix_slider_.setBounds(bound.removeFromLeft(t_width));
        dyn_smooth_label_.setBounds(bound.removeFromLeft(slider_width));
        dyn_smooth_slider_.setBounds(bound.removeFromLeft(t_width));
    }

    void ExtraDynamicPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void ExtraDynamicPanel::updateBand() {
        if (base_.getSelectedBand() < zlp::kBandNum) {
            const auto band_s = std::to_string(base_.getSelectedBand());
            rms_length_attach_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                rms_length_slider_.getSlider(), p_ref_.parameters_, zlp::PDynamicRMSLength::kID + band_s, updater_);
            rms_mix_attach_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                rms_mix_slider_.getSlider(), p_ref_.parameters_, zlp::PDynamicRMSMix::kID + band_s, updater_);
            dyn_smooth_attach_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                dyn_smooth_slider_.getSlider(), p_ref_.parameters_, zlp::PDynamicSmooth::kID + band_s, updater_);
        } else {
            rms_length_attach_.reset();
            rms_mix_attach_.reset();
            dyn_smooth_attach_.reset();
        }
    }

    void ExtraDynamicPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kDynamicExtraPanel, property)) {
            const auto extra_visible =
                static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kDynamicExtraPanel)) > 0.5;
            setVisible(extra_visible);
        }
    }
}
