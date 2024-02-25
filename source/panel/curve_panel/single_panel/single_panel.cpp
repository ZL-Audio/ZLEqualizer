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
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base,
                             zlDSP::Controller<double> &controller)
        : idx(bandIdx), parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base), controllerRef(controller),
          filter(controller.getFilter(idx)),
          baseF(controller.getFilter(idx).getBaseFilter()),
          targetF(controller.getFilter(idx).getTargetFilter()),
          sidePanel(bandIdx, parameters, parametersNA, base, controller) {
        curvePath.preallocateSpace(zlIIR::frequencies.size() * 3 + 12);
        shadowPath.preallocateSpace(zlIIR::frequencies.size() * 3 + 12);
        dynPath.preallocateSpace(zlIIR::frequencies.size() * 6 + 12);

        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        juce::ignoreUnused(controllerRef);
        skipRepaint.store(true);
        parameterChanged(zlDSP::dynamicON::ID + suffix,
                         parametersRef.getRawParameterValue(zlDSP::dynamicON::ID + suffix)->load());
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        parameterChanged(zlState::active::ID + suffix,
                         parametersNARef.getRawParameterValue(zlState::active::ID + suffix)->load());

        for (auto &id: changeIDs) {
            parametersRef.addParameterListener(id + suffix, this);
        }
        parametersRef.addParameterListener(zlDSP::scale::ID, this);
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.addParameterListener(zlState::active::ID + suffix, this);

        setInterceptsMouseClicks(false, false);
        // setBufferedToImage(true);
        addAndMakeVisible(sidePanel);
        skipRepaint.store(false);
    }

    SinglePanel::~SinglePanel() {
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        for (auto &id: changeIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        parametersRef.removeParameterListener(zlDSP::scale::ID, this);
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlState::active::ID + suffix, this);
    }

    void SinglePanel::paint(juce::Graphics &g) {
        if (!actived.load()) {
            return;
        }
        colour = uiBase.getColorMap1(idx);
        const auto thickness = selected.load() ? uiBase.getFontSize() * 0.15f : uiBase.getFontSize() * 0.075f;
        g.setColour(colour);
        // draw curve
        {
            juce::ScopedLock lock(curvePathLock);
            g.strokePath(curvePath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));
        }
        // draw shadow
        if (selected.load()) {
            g.setColour(colour.withMultipliedAlpha(0.125f));
            juce::ScopedLock lock(shadowPathLock);
            g.fillPath(shadowPath);
        }
        // draw dynamic shadow
        if (dynON.load()) {
            if (selected.load()) {
                g.setColour(colour.withMultipliedAlpha(0.25f));
            } else {
                g.setColour(colour.withMultipliedAlpha(0.125f));
            }
            juce::ScopedLock lock(dynPathLock);
            g.fillPath(dynPath);
        }
        // draw the line between the curve and the button
        {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
            g.setColour(colour);
            switch (baseF.getFilterType()) {
                case zlIIR::FilterType::peak:
                case zlIIR::FilterType::bandShelf: {
                    const auto x1 = freqToX(static_cast<double>(baseF.getFreq()), bound);
                    const auto y1 = dbToY(static_cast<float>(baseF.getDB(baseF.getFreq())), maximumDB.load(), bound);
                    const auto y2 = dbToY(
                        parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::gain::ID, idx))->load(),
                        maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, uiBase.getFontSize() * 0.065f);
                    break;
                }
                case zlIIR::FilterType::lowShelf:
                case zlIIR::FilterType::highShelf:
                case zlIIR::FilterType::tiltShelf: {
                    const auto x1 = freqToX(static_cast<double>(baseF.getFreq()), bound);
                    const auto y1 = dbToY(static_cast<float>(baseF.getDB(baseF.getFreq())), maximumDB.load(), bound);
                    const auto y2 = dbToY(
                        parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::gain::ID, idx))->load() / 2,
                        maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, uiBase.getFontSize() * 0.065f);
                    break;
                }
                case zlIIR::FilterType::notch:
                case zlIIR::FilterType::lowPass:
                case zlIIR::FilterType::highPass:
                case zlIIR::FilterType::bandPass: {
                    const auto x1 = freqToX(static_cast<double>(baseF.getFreq()), bound);
                    const auto y1 = dbToY(static_cast<float>(baseF.getDB(baseF.getFreq())), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(0), maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, uiBase.getFontSize() * 0.065f);
                    break;
                }
            }
        }
    }

    void SinglePanel::resized() {
        sidePanel.setBounds(getLocalBounds());
    }

    void SinglePanel::drawCurve(juce::Path &path,
                                const std::array<double, zlIIR::frequencies.size()> &dBs, bool reverse,
                                const bool startPath) {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        const auto maxDB = maximumDB.load();
        if (reverse) {
            auto y0 = 0.f;
            for (size_t j = zlIIR::frequencies.size(); j > 0; --j) {
                const auto i = j - 1;
                const auto x = indexToX(i, bound);
                const auto y = dbToY(static_cast<float>(dBs[i]), maxDB, bound);
                if (i == zlIIR::frequencies.size() - 1 && startPath) {
                    path.startNewSubPath(x, y);
                    y0 = y;
                } else if (std::abs(y - y0) >= 0.125f || i == 0) {
                    path.lineTo(x, y);
                    y0 = y;
                }
            }
        } else {
            auto y0 = 0.f;
            for (size_t i = 0; i < zlIIR::frequencies.size(); ++i) {
                const auto x = indexToX(i, bound);
                const auto y = dbToY(static_cast<float>(dBs[i]), maxDB, bound);
                if (i == 0 && startPath) {
                    path.startNewSubPath(x, y);
                    y0 = y;
                } else if (std::abs(y - y0) >= 0.125f || i == zlIIR::frequencies.size() - 1) {
                    path.lineTo(x, y);
                    y0 = y;
                }
            }
        }
    }

    void SinglePanel::parameterChanged(const juce::String &parameterID, float newValue) {
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

    void SinglePanel::handleAsyncUpdate() {
        if (!skipRepaint.load()) {
            updatePaths();
            repaint();
        }
    }

    void SinglePanel::updatePaths() {
        // draw curve
        {
            juce::ScopedLock lock(curvePathLock);
            baseF.updateDBs();
            curvePath.clear();
            drawCurve(curvePath, baseF.getDBs());
        }
        // draw shadow
        {
            juce::ScopedLock lock(shadowPathLock);
            shadowPath.clear();
            drawCurve(shadowPath, baseF.getDBs());
            if (selected.load()) {
                switch (baseF.getFilterType()) {
                    case zlIIR::FilterType::peak:
                    case zlIIR::FilterType::lowShelf:
                    case zlIIR::FilterType::highShelf:
                    case zlIIR::FilterType::notch:
                    case zlIIR::FilterType::bandShelf:
                    case zlIIR::FilterType::tiltShelf: {
                        const auto bound = getLocalBounds().toFloat();
                        shadowPath.lineTo(bound.getRight(), bound.getCentreY());
                        shadowPath.lineTo(bound.getX(), bound.getCentreY());
                        shadowPath.closeSubPath();
                        break;
                    }
                    case zlIIR::FilterType::lowPass:
                    case zlIIR::FilterType::highPass:
                    case zlIIR::FilterType::bandPass: {
                        const auto bound = getLocalBounds().toFloat();
                        shadowPath.lineTo(bound.getBottomRight());
                        shadowPath.lineTo(bound.getBottomLeft());
                        shadowPath.closeSubPath();
                        break;
                    }
                }
            }
        }
        // draw dynamic shadow
        {
            juce::ScopedLock lock(dynPathLock);
            dynPath.clear();
            drawCurve(dynPath, baseF.getDBs());
            if (dynON.load()) {
                targetF.updateDBs();
                drawCurve(dynPath, targetF.getDBs(), true, false);
                dynPath.closeSubPath();
            }
        }
    }
} // zlPanel
