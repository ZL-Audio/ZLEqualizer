// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "setting_panel.hpp"
#include "../../state/state.hpp"
#include "../panel_definitons.hpp"

namespace zlpanel {
    SettingPanel::SettingPanel(PluginProcessor &p, zlgui::UIBase &base,
                               const juce::String &label, zlgui::BoxIdx idx)
        : parameters_ref_(p.parameters_),
          parameters_NA_ref_(p.parameters_NA_),
          ui_base_(base), setting_name_(label), box_idx_(idx) {
        juce::ignoreUnused(parameters_ref_, parameters_NA_ref_);
        setBufferedToImage(true);

        ui_base_.getBoxTree().addListener(this);
    }

    SettingPanel::~SettingPanel() {
        ui_base_.getBoxTree().removeListener(this);
        stopTimer(0);
        stopTimer(1);
    }

    void SettingPanel::paint(juce::Graphics &g) {
        const auto is_box_open = static_cast<bool>(ui_base_.getBoxProperty(box_idx_));
        if (is_box_open) {
            g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.25f));
        } else {
            g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.125f));
        }
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 ui_base_.getFontSize() * .5f, ui_base_.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
        g.setFont(ui_base_.getFontSize() * 1.375f);
        if (is_box_open) {
            g.setColour(ui_base_.getTextColor());
        } else {
            g.setColour(ui_base_.getTextColor().withAlpha(.75f));
        }
        g.drawText(setting_name_, bound, juce::Justification::centred);
    }

    void SettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer(0);
        if (isTimerRunning(1)) return;
        if (ui_base_.getBoxProperty(box_idx_)) {
            ui_base_.setBoxProperty(box_idx_, false);
        } else {
            ui_base_.openOneBox(box_idx_);
        }
    }

    void SettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (!ui_base_.getBoxProperty(box_idx_)) {
            startTimer(0, 100);
            startTimer(1, 500);
        }
    }

    void SettingPanel::mouseExit(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer(0);
        stopTimer(1);
    }

    void SettingPanel::timerCallback(const int timerID) {
        if (timerID == 0) {
            ui_base_.openOneBox(box_idx_);
            stopTimer(0);
        } else if (timerID == 1) {
            stopTimer(1);
        }
    }

    void SettingPanel::valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                                const juce::Identifier &property) {
        juce::ignoreUnused(tree_whose_property_has_changed);
        if (zlgui::UIBase::isBoxProperty(box_idx_, property)) repaint();
    }
} // zlpanel
