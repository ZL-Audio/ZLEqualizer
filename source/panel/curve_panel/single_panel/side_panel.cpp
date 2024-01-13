// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "side_panel.hpp"

namespace zlPanel {
    SidePanel::SidePanel(size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base,
                         zlDSP::Controller<double> &controller)
        : idx(bandIdx),
          parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          sideF(controller.getFilter(bandIdx).getSideFilter()) {
        setBufferedToImage(true);
        colour = uiBase.getColorMap1(idx);

        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        parameterChanged(zlState::active::ID + suffix,
                         parametersNARef.getRawParameterValue(zlState::active::ID + suffix)->load());

        for (auto &id: changeIDs) {
            parametersRef.addParameterListener(id + suffix, this);
        }
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.addParameterListener(zlState::active::ID + suffix, this);
    }

    SidePanel::~SidePanel() {
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        for (auto &id: changeIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlState::active::ID + suffix, this);
    }

    void SidePanel::paint(juce::Graphics &g) {
        if (!selected.load() || !actived.load() || !dynON.load()) {
            return;
        }
        const auto q = sideF.getQ(), freq = sideF.getFreq();
        const auto bw = 2 * std::asinh(0.5f / q) / std::log(2.f);
        const auto scale = std::pow(2.f, bw / 2.f);
        const auto freq1 = freq / scale, freq2 = freq * scale;

        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 4 * uiBase.getFontSize());
        const auto x1 = std::log(freq1 / 10.f) / std::log(2200.f) * bound.getWidth();
        const auto x2 = std::log(freq2 / 10.f) / std::log(2200.f) * bound.getWidth();

        const auto thickness = uiBase.getFontSize() * 0.15f;
        g.setColour(colour);
        g.drawLine(static_cast<float>(x1), bound.getBottom(), static_cast<float>(x2), bound.getBottom(), thickness);
    }

    void SidePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            selected.store(static_cast<size_t>(newValue) == idx);
        } else {
            const auto id = parameterID.dropLastCharacters(2);
            if (id == zlState::active::ID) {
                actived.store(static_cast<bool>(newValue));
            } else if (id == zlDSP::dynamicON::ID) {
                dynON.store(static_cast<bool>(newValue));
            }
        }
        triggerAsyncUpdate();
    }

    void SidePanel::handleAsyncUpdate() {
        repaint();
    }
} // zlPanel
