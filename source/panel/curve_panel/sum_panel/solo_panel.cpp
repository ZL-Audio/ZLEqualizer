// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlPanel {
    SoloPanel::SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base,
                         zlDSP::Controller<double> &controller)
        : parametersRef(parameters), uiBase(base),
          soloF(controller.getSoloFilter()),
          controllerRef(controller) {
        juce::ignoreUnused(parametersNA);
        // setBufferedToImage(true);
        for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
            const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
            for (auto &id: changeIDs) {
                parametersRef.addParameterListener(id + suffix, this);
            }
        }
        triggerAsyncUpdate();
    }

    SoloPanel::~SoloPanel() {
        for (size_t idx = 0; idx < zlState::bandNUM; ++idx) {
            const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
            for (auto &id: changeIDs) {
                parametersRef.removeParameterListener(id + suffix, this);
            }
        }
    }

    void SoloPanel::paint(juce::Graphics &g) {
        if (!controllerRef.getSolo()) {
            return;
        }
        if (soloF.getMagOutdated(false)) {
            handleAsyncUpdate();
        } else if (toRepaint.exchange(false)) {
            handleAsyncUpdate();
        }
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight());
        const auto x1 = scale1.load() * bound.getWidth();
        const auto x2 = scale2.load() * bound.getWidth();

        g.setColour(uiBase.getTextInactiveColor());
        const auto width = bound.getWidth();
        const auto leftArea = bound.removeFromLeft(x1);
        const auto rightArea = bound.removeFromRight(width - x2);

        juce::Path mask;
        bound = getLocalBounds().toFloat();
        mask.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * 0.5f, uiBase.getFontSize() * 0.5f,
                                 true, false, true, false);
        g.saveState();
        g.reduceClipRegion(mask);
        g.fillRect(leftArea);
        g.fillRect(rightArea);
        g.restoreState();
    }

    void SoloPanel::checkRepaint() {
        if (soloF.getMagOutdated()) {
            soloF.setMagOutdated(false);
            handleAsyncUpdate();
            repaint();
        } else if (toRepaint.load()) {
            toRepaint.store(false);
            handleAsyncUpdate();
            repaint();
        }
    }

    void SoloPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        toRepaint.store(true);
    }

    void SoloPanel::handleAsyncUpdate() {
        const auto q = soloF.getQ(), freq = soloF.getFreq();
        const auto bw = 2 * std::asinh(0.5f / q) / std::log(2.f);
        const auto scale = std::pow(2.f, bw / 2.f);
        const auto freq1 = freq / scale, freq2 = freq * scale;

        scale1.store(std::log(static_cast<float>(freq1) / 10.f) / std::log(2200.f));
        scale2.store(std::log(static_cast<float>(freq2) / 10.f) / std::log(2200.f));
    }
} // zlPanel
