// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "loudness_display.hpp"

namespace zlpanel {
    LoudnessDisplay::LoudnessDisplay(PluginProcessor &p, zlgui::UIBase &base)
        : processorRef(p), uiBase(base) {
        for (size_t i = 0; i < zlstate::bandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            isThresholdAutoParas[i] = processorRef.parameters.getParameter(zlp::dynamicLearn::ID + suffix);
            isDynamicOnParas[i] = processorRef.parameters.getParameter(zlp::dynamicON::ID + suffix);
        }
        bandIdxPara = processorRef.parametersNA.getParameter(zlstate::selectedBandIdx::ID);
        lookAndFeelChanged();
    }

    void LoudnessDisplay::paint(juce::Graphics &g) {
        const auto loudness = processorRef.getController().getSideLoudness(bandIdx);
        const auto p = 1. + std::clamp(loudness, -80.0, 0.0) / 80;
        auto bound = getLocalBounds().toFloat();
        bound = bound.withWidth(bound.getWidth() * static_cast<float>(p));
        g.setColour(colour);
        g.fillRect(bound);
    }

    void LoudnessDisplay::checkVisible() {
        bandIdx = static_cast<size_t>(bandIdxPara->convertFrom0to1(bandIdxPara->getValue()));
        const auto f = (isThresholdAutoParas[bandIdx]->getValue() < 0.5f)
                              && (isDynamicOnParas[bandIdx]->getValue() > 0.5f);
        setVisible(f && shouldVisible && colour.getFloatAlpha() > 0.005f);
    }

    void LoudnessDisplay::lookAndFeelChanged() {
        colour = uiBase.getColourByIdx(zlgui::sideLoudnessColour);
    }

    void LoudnessDisplay::updateVisible(const bool x) {
        shouldVisible = x;
    }
} // zlpanel
