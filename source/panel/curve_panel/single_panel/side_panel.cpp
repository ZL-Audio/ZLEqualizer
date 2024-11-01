// Copyright (C) 2024 - zsliu98
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
        setInterceptsMouseClicks(false, false);
        const std::string suffix = zlDSP::appendSuffix("", idx);
        skipRepaint.store(true);
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
        parameterChanged(zlDSP::sideFreq::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::sideFreq::ID + suffix)->load());
        parameterChanged(zlDSP::sideQ::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::sideQ::ID + suffix)->load());
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        parameterChanged(zlState::active::ID + suffix,
                         parametersNARef.getRawParameterValue(zlState::active::ID + suffix)->load());
        skipRepaint.store(false);

        for (auto &id: changeIDs) {
            parametersRef.addParameterListener(id + suffix, this);
        }
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.addParameterListener(zlState::active::ID + suffix, this);
        update();
        lookAndFeelChanged();
    }

    SidePanel::~SidePanel() {
        const std::string suffix = zlDSP::appendSuffix("", idx);
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
        if (toUpdate.exchange(false)) {
            update();
        }
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 4 * uiBase.getFontSize());
        const auto x1 = scale1.load() * bound.getWidth();
        const auto x2 = scale2.load() * bound.getWidth();

        const auto thickness = uiBase.getFontSize() * 0.15f;
        g.setColour(colour);
        g.drawLine(x1, bound.getBottom(), x2, bound.getBottom(), thickness);
    }

    bool SidePanel::checkRepaint() {
        if (toRepaint.exchange(false)) {
            return true;
        }
        return false;
    }

    void SidePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            selected.store(static_cast<size_t>(newValue) == idx);
        } else {
            if (parameterID.startsWith(zlState::active::ID)) {
                actived.store(newValue > .5f);
            } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
                dynON.store(newValue > .5f);
            } else if (parameterID.startsWith(zlDSP::sideFreq::ID)) {
                sideFreq.store(newValue);
                toUpdate.store(true);
            } else if (parameterID.startsWith(zlDSP::sideQ::ID)) {
                sideQ.store(newValue);
                toUpdate.store(true);
            }
        }
        if (!skipRepaint.load()) {
            toRepaint.store(true);
        }
    }

    void SidePanel::update() {
        const auto q = sideQ.load(), freq = sideFreq.load();
        const auto bw = 2 * std::asinh(0.5f / q) / std::log(2.f);
        const auto scale = std::pow(2.f, bw / 2.f);
        const auto freq1 = freq / scale, freq2 = freq * scale;

        scale1.store(std::log(static_cast<float>(freq1) / 10.f) / std::log(2200.f));
        scale2.store(std::log(static_cast<float>(freq2) / 10.f) / std::log(2200.f));
    }

    void SidePanel::lookAndFeelChanged() {
        colour = uiBase.getColorMap1(idx);
    }
} // zlPanel
