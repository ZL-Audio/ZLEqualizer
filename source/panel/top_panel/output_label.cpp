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
        scale_ref_(*p.parameters_.getRawParameterValue(zlp::PGainScale::kID)) {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.);
    }

    void OutputLabel::paint(juce::Graphics& g) {
        const auto padding = getPaddingSize(base_.getFontSize());

        if (static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kOutputPanel)) > .5) {
            const auto bound = getLocalBounds().reduced(padding, padding / 2).toFloat();
            juce::Path path;
            path.addRoundedRectangle(bound, static_cast<float>(padding));
            const juce::DropShadow shadow{base_.getBrightShadowColour(), padding, {0, 0}};
            shadow.drawForPath(g, path);
            g.setColour(base_.getBackgroundColour());
            g.fillPath(path);
        }

        auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getTextColour());
        g.setFont(1.5f * base_.getFontSize());
        g.drawText(floatToStringSnprintf(c_scale_, 0) + "%",
                   bound.removeFromLeft(bound.getWidth() * .5f),
                   juce::Justification::centred);
        g.drawText(floatToStringSnprintf(static_cast<float>(c_gain_db_), 1) + "dB",
                   bound,
                   juce::Justification::centred);
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
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, f > .5 ? 0. : 1.);
        repaint();
    }

    void OutputLabel::checkUpdate() {
        const auto scale = scale_ref_.load(std::memory_order_relaxed);
        const auto gain_db = zldsp::chore::gainToDecibels(p_ref_.getController().getDisplayedGain());
        if (std::abs(scale - c_scale_) > 1e-3 || std::abs(gain_db - c_gain_db_) > 1e-3) {
            c_scale_ = scale;
            c_gain_db_ = gain_db;
            repaint();
        }
    }
}
