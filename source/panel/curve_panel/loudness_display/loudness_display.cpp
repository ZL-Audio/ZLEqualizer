// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "loudness_display.hpp"

namespace zlPanel {
    LoudnessDisplay::LoudnessDisplay(PluginProcessor &p, zlInterface::UIBase &base)
        : processorRef(p), uiBase(base) {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            const auto suffix = zlDSP::appendSuffix("", i);
            isThresholdAutoParas[i] = processorRef.parameters.getParameter(zlDSP::dynamicLearn::ID + suffix);
            isDynamicOnParas[i] = processorRef.parameters.getParameter(zlDSP::dynamicON::ID + suffix);
        }
    }

    void LoudnessDisplay::paint(juce::Graphics &g) {
        const auto loudness = processorRef.getController().getSideLoudness(bandIdx);
        const auto p = 1. + std::clamp(loudness, -80.0, 0.0) / 80;
        auto bound = getLocalBounds().toFloat();
        bound = bound.withWidth(bound.getWidth() * static_cast<float>(p));
        g.setColour(uiBase.getTextColor().withAlpha(.33f));
        g.fillRect(bound);
    }

    void LoudnessDisplay::checkRepaint() {
        const auto currentTime = juce::Time::getCurrentTime();
        if ((currentTime - previousTime).inMilliseconds() >= 33) {
            const bool newShouldPaint = (isThresholdAutoParas[bandIdx]->getValue() < 0.5f)
                                        && (isDynamicOnParas[bandIdx]->getValue() > 0.5f);
            if (shouldPaint || shouldPaint != newShouldPaint) {
                shouldPaint = newShouldPaint;
            }
            previousTime = currentTime;
        }
    }
} // zlPanel
