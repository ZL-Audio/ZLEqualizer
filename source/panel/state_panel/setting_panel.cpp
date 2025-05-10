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
        : parameters_ref_(p.parameters),
          parameters_NA_ref_(p.parameters_NA),
          ui_base_(base), name(label), mIdx(idx) {
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
        const bool isBoxOpen = static_cast<bool>(ui_base_.getBoxProperty(mIdx));
        if (isBoxOpen) {
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
        if (isBoxOpen) {
            g.setColour(ui_base_.getTextColor());
        } else {
            g.setColour(ui_base_.getTextColor().withAlpha(.75f));
        }
        g.drawText(name, bound, juce::Justification::centred);
    }

    void SettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer(0);
        if (isTimerRunning(1)) return;
        if (ui_base_.getBoxProperty(mIdx)) {
            ui_base_.setBoxProperty(mIdx, false);
        } else {
            ui_base_.openOneBox(mIdx);
        }
    }

    void SettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (!ui_base_.getBoxProperty(mIdx)) {
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
            ui_base_.openOneBox(mIdx);
            stopTimer(0);
        } else if (timerID == 1) {
            stopTimer(1);
        }
    }

    void SettingPanel::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                const juce::Identifier &property) {
        juce::ignoreUnused(treeWhosePropertyHasChanged);
        if (zlgui::UIBase::isBoxProperty(mIdx, property)) repaint();
    }
} // zlpanel
