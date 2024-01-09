// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlPanel {
    SinglePanel::SinglePanel(const size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base,
                             zlDSP::Controller<float> &controller)
        : idx(bandIdx), parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base), controllerRef(controller),
          filter(controller.getFilter(idx)),
          baseF(controller.getFilter(idx).getBaseFilter()),
          targetF(controller.getFilter(idx).getTargetFilter()) {
        path.preallocateSpace(zlIIR::frequencies.size() * 6);

        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        juce::ignoreUnused(controllerRef);
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

        setBufferedToImage(true);
        colour = uiBase.getColorMap1(idx);
    }

    SinglePanel::~SinglePanel() {
        path.clear();
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        for (auto &id: changeIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlState::active::ID + suffix, this);
    }

    void SinglePanel::paint(juce::Graphics &g) {
        if (!actived.load()) {
            return;
        }
        path.clear();
        const auto thickness = selected.load() ? uiBase.getFontSize() * 0.15f : uiBase.getFontSize() * 0.075f;
        baseF.updateDBs();
        drawCurve(baseF.getDBs());
        g.setColour(colour);
        g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        if (selected.load()) {
            switch (baseF.getFilterType()) {
                case zlIIR::FilterType::peak:
                case zlIIR::FilterType::lowShelf:
                case zlIIR::FilterType::highShelf:
                case zlIIR::FilterType::notch:
                case zlIIR::FilterType::bandShelf:
                case zlIIR::FilterType::tiltShelf: {
                    const auto bound = getLocalBounds().toFloat();
                    path.lineTo(bound.getRight(), bound.getCentreY());
                    path.lineTo(bound.getX(), bound.getCentreY());
                    path.closeSubPath();
                    break;
                }
                case zlIIR::FilterType::lowPass:
                case zlIIR::FilterType::highPass:
                case zlIIR::FilterType::bandPass: {
                    const auto bound = getLocalBounds().toFloat();
                    path.lineTo(bound.getBottomRight());
                    path.lineTo(bound.getBottomLeft());
                    path.closeSubPath();
                    break;
                }
            }
            g.setColour(colour.withMultipliedAlpha(0.125f));
            g.fillPath(path);
            path.clear();
            drawCurve(baseF.getDBs());
        }
        if (dynON.load()) {
            targetF.updateDBs();
            drawCurve(targetF.getDBs(), true, false);
            path.closeSubPath();
            g.setColour(colour.withMultipliedAlpha(0.25f));
            // g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
            //                                         juce::PathStrokeType::rounded));
            g.fillPath(path);
        }
    }

    void SinglePanel::drawCurve(const std::array<float, zlIIR::frequencies.size()> &dBs, bool reverse, bool startPath) {
        const auto bound = getLocalBounds().toFloat();
        if (reverse) {
            for (size_t j = zlIIR::frequencies.size(); j > 0; --j) {
                const auto i = j - 1;
                const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.
                               getWidth();
                const auto y = (dBs[i] / (-60) + 0.5f) * bound.getHeight();
                if (i == 0 && startPath) {
                    path.startNewSubPath(x, y);
                } else {
                    path.lineTo(x, y);
                }
            }
        } else {
            for (size_t i = 0; i < zlIIR::frequencies.size(); ++i) {
                const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.
                               getWidth();
                const auto y = (dBs[i] / (-60) + 0.5f) * bound.getHeight();
                if (i == 0 && startPath) {
                    path.startNewSubPath(x, y);
                } else {
                    path.lineTo(x, y);
                }
            }
        }
    }

    void SinglePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            selected.store(static_cast<size_t>(newValue) == idx);
        } else {
            const auto id = parameterID.dropLastCharacters(2);
            // const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
            if (id == zlState::active::ID) {
                actived.store(static_cast<bool>(newValue));
            } else if (id == zlDSP::dynamicON::ID) {
                dynON.store(static_cast<bool>(newValue));
            }
        }
        triggerAsyncUpdate();
    }

    void SinglePanel::handleAsyncUpdate() {
        repaint();
    }
} // zlPanel
