// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlPanel {
    SoloPanel::SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base,
                         zlDSP::Controller<double> &controller)
        : parametersRef(parameters), uiBase(base),
          soloF(controller.getSoloFilter()),
          controllerRef(controller) {
        juce::ignoreUnused(parametersRef, parametersNA);
        handleAsyncUpdate();
    }

    SoloPanel::~SoloPanel() = default;
    void SoloPanel::paint(juce::Graphics &g) {
        if (!controllerRef.getSolo()) {
            return;
        }
        if (std::abs(soloF.getFreq() - soloFreq) >= 0.001 || std::abs(soloF.getQ() - soloQ) >= 0.001) {
            handleAsyncUpdate();
        }
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight());
        const auto x1 = scale1 * bound.getWidth();
        const auto x2 = scale2 * bound.getWidth();

        const auto width = bound.getWidth();
        const auto leftArea = bound.removeFromLeft(x1);
        const auto rightArea = bound.removeFromRight(width - x2);

        g.setColour(uiBase.getTextInactiveColor());
        g.fillRect(leftArea);
        g.fillRect(rightArea);
    }

    void SoloPanel::handleAsyncUpdate() {
        soloFreq = soloF.getFreq();
        soloQ = soloF.getQ();
        const auto bw = 2 * std::asinh(0.5f / soloQ) / std::log(2.f);
        const auto scale = std::pow(2.f, bw / 2.f);
        const auto freq1 = soloFreq / scale, freq2 = soloFreq * scale;

        scale1 = std::log(static_cast<float>(freq1) / 10.f) / std::log(2200.f);
        scale2 = std::log(static_cast<float>(freq2) / 10.f) / std::log(2200.f);
    }
} // zlPanel
