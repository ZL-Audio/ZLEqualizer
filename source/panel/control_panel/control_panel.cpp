// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

#include "../../state/state_definitions.hpp"

namespace zlpanel {
    ControlPanel::ControlPanel(PluginProcessor &p,
                               zlgui::UIBase &base)
        : parametersRef(p.parameters), parametersNARef(p.parametersNA), uiBase(base),
          leftControlPanel(p, base),
          rightControlPanel(p, base),
          matchControlPanel(p, base) {
        addAndMakeVisible(leftControlPanel);
        addChildComponent(rightControlPanel);
        addChildComponent(matchControlPanel);
        for (size_t i = 0; i < zlstate::bandNUM; ++i) {
            const auto idx = zlp::appendSuffix(zlp::dynamicON::ID, i);
            dynamicON[i].store(parametersRef.getRawParameterValue(idx)->load() > .5f);
            parametersRef.addParameterListener(idx, this);
        }
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        parametersNARef.addParameterListener(zlstate::selectedBandIdx::ID, this);

        setOpaque(true);
    }

    ControlPanel::~ControlPanel() {
        parametersNARef.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        for (size_t i = 0; i < zlstate::bandNUM; ++i) {
            const auto idx = zlp::appendSuffix(zlp::dynamicON::ID, i);
            parametersRef.removeParameterListener(idx, this);
        }
    }

    void ControlPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
    }

    void ControlPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        const auto actualBound = uiBase.getRoundedShadowRectangleArea(bound, uiBase.getFontSize(), {});

        const auto leftWidth = actualBound.getWidth() * (33.f / 63.f) + (bound.getWidth() - actualBound.getWidth()) *
                               .5f;
        auto rightBound = getLocalBounds();
        const auto leftBound = rightBound.removeFromLeft(juce::roundToInt(leftWidth));
        leftControlPanel.setBounds(leftBound);
        rightControlPanel.setBounds(rightBound);
        matchControlPanel.setBounds(getLocalBounds());
    }

    void ControlPanel::parameterChanged(const juce::String &parameterID, const float newValue) {
        if (parameterID == zlstate::selectedBandIdx::ID) {
            bandIdx.store(static_cast<size_t>(newValue));
            triggerAsyncUpdate();
        } else {
            const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
            dynamicON[idx].store(newValue > .5f);
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void ControlPanel::handleAsyncUpdate() {
        const auto idx = bandIdx.load();
        leftControlPanel.attachGroup(idx);
        rightControlPanel.attachGroup(idx);
        const auto f = dynamicON[idx].load();
        leftControlPanel.getDynamicAutoButton().setVisible(f);
        rightControlPanel.setVisible(f);
    }
} // zlpanel
