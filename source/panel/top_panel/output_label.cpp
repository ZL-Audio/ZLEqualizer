// Copyright (C) 2025 - zsliu98
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
        scale_ref_(*p.parameters_.getRawParameterValue(zlp::PGainScale::kID)) {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.);

        control_background_.setInterceptsMouseClicks(false, false);
        control_background_.setBufferedToImage(true);
        addChildComponent(control_background_);

        label_laf_.setFontScale(1.5f);

        scale_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setJustificationType(juce::Justification::centredLeft);
        for (auto& l:{&gain_label_, &scale_label_}) {
            l->setInterceptsMouseClicks(false, false);
            l->setLookAndFeel(&label_laf_);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        setAlpha(.5f);
        setInterceptsMouseClicks(true, false);

        base_.getPanelValueTree().addListener(this);
    }

    OutputLabel::~OutputLabel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void OutputLabel::resized() {
        const auto padding = 2 * getPaddingSize(base_.getFontSize());
        auto bound = getLocalBounds();
        control_background_.setBounds(0, -padding, bound.getWidth(), bound.getHeight() + padding + padding / 4);
        scale_label_.setBounds(bound.removeFromLeft(bound.getWidth() / 2));
        gain_label_.setBounds(bound);
    }

    void OutputLabel::repaintCallbackSlow() {
        repaint_count_ += 1;
        if (repaint_count_ >= 4) {
            repaint_count_ = 0;
            checkUpdate();
        }
    }

    void OutputLabel::mouseDown(const juce::MouseEvent&) {
        const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel));
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, f < .5 ? 1. : 0.);
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
            setAlpha(f > .5 ? 1.f : .5f);
        }
    }
}
