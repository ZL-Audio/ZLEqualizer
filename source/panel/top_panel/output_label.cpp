// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_label.hpp"

namespace zlpanel {
    OutputLabel::OutputLabel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        control_background_(base),
        label_laf_(base),
        scale_ref_(*p.parameters_.getRawParameterValue(zlp::PGainScale::kID)),
        gain_slider_(base_, ""),
        gain_attach_(gain_slider_, p.parameters_,
                     zlp::POutputGain::kID, updater_),
        scale_slider_(base_, ""),
        scale_attach_(scale_slider_, p.parameters_,
                      zlp::PGainScale::kID, updater_) {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.);

        control_background_.setInterceptsMouseClicks(false, false);
        control_background_.setBufferedToImage(true);
        addChildComponent(control_background_);

        label_laf_.setFontScale(1.5f);

        scale_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setJustificationType(juce::Justification::centredLeft);
        for (auto& l : {&gain_label_, &scale_label_}) {
            l->setInterceptsMouseClicks(false, false);
            l->setLookAndFeel(&label_laf_);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        addChildComponent(gain_slider_);
        addChildComponent(scale_slider_);

        setAlpha(.5f);
        setInterceptsMouseClicks(true, false);

        base_.getPanelValueTree().addListener(this);
    }

    OutputLabel::~OutputLabel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void OutputLabel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = 2 * getPaddingSize(font_size);
        auto bound = getLocalBounds();
        control_background_.setBounds(0, -padding, bound.getWidth(), bound.getHeight() + padding + padding / 4);
        scale_label_.setBounds(bound.removeFromLeft(bound.getWidth() / 2));
        gain_label_.setBounds(bound);

        gain_slider_.setBounds(getLocalBounds());
        scale_slider_.setBounds(getLocalBounds());
        const auto dragging_distance = getSliderDraggingDistance(font_size);
        gain_slider_.setMouseDragSensitivity(dragging_distance);
        scale_slider_.setMouseDragSensitivity(dragging_distance);
    }

    void OutputLabel::repaintCallbackSlow() {
        updater_.updateComponents();
        checkUpdate();
    }

    void OutputLabel::mouseDown(const juce::MouseEvent&) {
        const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel));
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, f < .5 ? 1. : 0.);
    }

    void OutputLabel::mouseEnter(const juce::MouseEvent&) {
        is_over_ = true;
        const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel));
        updateAlpha(f > .5);
    }

    void OutputLabel::mouseExit(const juce::MouseEvent&) {
        is_over_ = false;
        const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel));
        updateAlpha(f > .5);
    }

    void OutputLabel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        if (event.mods.isCommandDown()) {
            scale_slider_.mouseWheelMove(event, wheel);
        } else {
            gain_slider_.mouseWheelMove(event, wheel);
        }
    }

    void OutputLabel::checkUpdate() {
        const auto scale = scale_ref_.load(std::memory_order_relaxed);
        auto gain_db = zldsp::chore::gainToDecibels(p_ref_.getController().getDisplayedGain());
        if (std::abs(scale - c_scale_) > 1e-3) {
            c_scale_ = scale;
            scale_label_.setText(floatToStringSnprintf(c_scale_, 0) + "%",
                                 juce::dontSendNotification);
            scale_label_.repaint();
        }
        if (std::abs(gain_db) < 1e-6) {
            gain_db = 0.0;
        }
        if (std::abs(gain_db - c_gain_db_) > 1e-3) {
            c_gain_db_ = gain_db;
            gain_label_.setText(floatToStringSnprintf(static_cast<float>(c_gain_db_), 1) + "dB",
                                juce::dontSendNotification);
            gain_label_.repaint();
        }
    }

    void OutputLabel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::kOutputPanel, property)) {
            const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel));
            control_background_.setVisible(f > .5);
            updateAlpha(f > .5);
        }
    }

    void OutputLabel::updateAlpha(const bool is_panel_open) {
        if (is_panel_open) {
            setAlpha(1.f);
        } else if (is_over_) {
            setAlpha(.75f);
        } else {
            setAlpha(.5f);
        }
    }
}
