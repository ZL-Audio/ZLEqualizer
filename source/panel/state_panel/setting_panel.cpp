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

namespace zlPanel {
    SettingPanel::SettingPanel(PluginProcessor &p, zlInterface::UIBase &base,
                               const juce::String &label, zlInterface::boxIdx idx)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base), name(label), mIdx(idx) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        setBufferedToImage(true);

        uiBase.getBoxTree().addListener(this);
    }

    SettingPanel::~SettingPanel() {
        uiBase.getBoxTree().removeListener(this);
        stopTimer(0);
        stopTimer(1);
    }

    void SettingPanel::paint(juce::Graphics &g) {
        const bool isBoxOpen = static_cast<bool>(uiBase.getBoxProperty(mIdx));
        if (isBoxOpen) {
            g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        } else {
            g.setColour(uiBase.getTextColor().withMultipliedAlpha(.125f));
        }
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
        g.setFont(uiBase.getFontSize() * 1.375f);
        if (isBoxOpen) {
            g.setColour(uiBase.getTextColor());
        } else {
            g.setColour(uiBase.getTextColor().withAlpha(.75f));
        }
        g.drawText(name, bound, juce::Justification::centred);
    }

    void SettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer(0);
        if (isTimerRunning(1)) return;
        if (uiBase.getBoxProperty(mIdx)) {
            uiBase.setBoxProperty(mIdx, false);
        } else {
            uiBase.openOneBox(mIdx);
        }
    }

    void SettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (!uiBase.getBoxProperty(mIdx)) {
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
            uiBase.openOneBox(mIdx);
            stopTimer(0);
        } else if (timerID == 1) {
            stopTimer(1);
        }
    }

    void SettingPanel::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                const juce::Identifier &property) {
        juce::ignoreUnused(treeWhosePropertyHasChanged);
        if (zlInterface::UIBase::isBoxProperty(mIdx, property)) repaint();
    }
} // zlPanel
