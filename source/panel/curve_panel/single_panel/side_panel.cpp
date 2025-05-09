// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "side_panel.hpp"

namespace zlpanel {
    SidePanel::SidePanel(const size_t bandIdx,
                         juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parametersNA,
                         zlgui::UIBase &base,
                         zlp::Controller<double> &controller,
                         zlgui::Dragger &sideDragger)
        : idx(bandIdx),
          parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          sideF(controller.getFilter(bandIdx).getSideFilter()),
          sideDraggerRef(sideDragger) {
        setInterceptsMouseClicks(false, false);
        const std::string suffix = zlp::appendSuffix("", idx);
        parameterChanged(zlp::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlp::dynamicON::ID + suffix)->load());
        parameterChanged(zlp::sideQ::ID + suffix,
                         parametersRef.getRawParameterValue(zlp::sideQ::ID + suffix)->load());
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());
        parameterChanged(zlstate::active::ID + suffix,
                         parametersNARef.getRawParameterValue(zlstate::active::ID + suffix)->load());

        for (auto &id: changeIDs) {
            parametersRef.addParameterListener(id + suffix, this);
        }
        parametersNARef.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parametersNARef.addParameterListener(zlstate::active::ID + suffix, this);
        lookAndFeelChanged();
    }

    SidePanel::~SidePanel() {
        const std::string suffix = zlp::appendSuffix("", idx);
        for (auto &id: changeIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        parametersNARef.removeParameterListener(zlstate::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlstate::active::ID + suffix, this);
    }

    void SidePanel::paint(juce::Graphics &g) {
        if (!isVisible()) {
            return;
        }
        const auto bound = getLocalBounds().toFloat();

        const auto x = sideDraggerRef.getButton().getBoundsInParent().toFloat().getCentreX();
        const auto thickness = uiBase.getFontSize() * 0.15f;
        g.setColour(colour);
        g.drawLine(x - currentBW, bound.getY(), x + currentBW, bound.getY(), thickness);
    }

    void SidePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlstate::selectedBandIdx::ID) {
            selected.store(static_cast<size_t>(newValue) == idx);
        } else {
            if (parameterID.startsWith(zlstate::active::ID)) {
                actived.store(newValue > .5f);
            } else if (parameterID.startsWith(zlp::dynamicON::ID)) {
                dynON.store(newValue > .5f);
            } else if (parameterID.startsWith(zlp::sideQ::ID)) {
                sideQ.store(newValue);
                toUpdate.store(true);
            }
        }
    }

    void SidePanel::updateDragger() {
        if (!selected.load() || !actived.load() || !dynON.load()) {
            setVisible(false);
            return;
        } else {
            setVisible(true);
        }
        if (toUpdate.exchange(false)) {
            const auto bw = std::asinh(0.5f / sideQ.load());
            currentBW = static_cast<float>(bw) / std::log(2200.f) * getLocalBounds().toFloat().getWidth();
        }
    }

    void SidePanel::lookAndFeelChanged() {
        colour = uiBase.getColorMap1(idx);
    }
} // zlpanel
