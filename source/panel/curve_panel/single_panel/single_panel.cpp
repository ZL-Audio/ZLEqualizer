// Copyright (C) 2024 - zsliu98
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
        curvePath.preallocateSpace(static_cast<int>(zlIIR::frequencies.size() * 3 + 12));
        shadowPath.preallocateSpace(static_cast<int>(zlIIR::frequencies.size() * 3 + 12));
        dynPath.preallocateSpace(static_cast<int>(zlIIR::frequencies.size() * 6 + 12));

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
        auto thickness = selected.load() ? uiBase.getFontSize() * 0.15f : uiBase.getFontSize() * 0.075f;
        thickness *= uiBase.getSingleCurveThickness();
        g.setColour(colour);
        // draw curve
        {
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::nonRealtime> pathLock(recentCurvePath);
            g.strokePath(*pathLock, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));
        }
        // draw shadow
        if (selected.load()) {
            g.setColour(colour.withMultipliedAlpha(0.125f));
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::nonRealtime> pathLock(recentShadowPath);
            g.fillPath(*pathLock);
        }
        // draw dynamic shadow
        if (dynON.load()) {
            if (selected.load()) {
                g.setColour(colour.withMultipliedAlpha(0.25f));
            } else {
                g.setColour(colour.withMultipliedAlpha(0.125f));
            }
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::nonRealtime> pathLock(recentDynPath);
            g.fillPath(*pathLock);
        }
        // draw the line between the curve and the button
        {
            const auto linkThickness = uiBase.getFontSize() * 0.065f * uiBase.getSingleCurveThickness();
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
            g.setColour(colour);
            switch (baseF.getFilterType()) {
                case zlIIR::FilterType::peak:
                case zlIIR::FilterType::bandShelf: {
                    const auto x1 = freqToX(baseFreq.load(), bound);
                    const auto y1 = dbToY(centeredDB.load(), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(baseGain.load()), maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, linkThickness);
                    break;
                }
                case zlIIR::FilterType::lowShelf:
                case zlIIR::FilterType::highShelf:
                case zlIIR::FilterType::tiltShelf: {
                    const auto x1 = freqToX(baseFreq.load(), bound);
                    const auto y1 = dbToY(centeredDB.load(), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(baseGain.load()) / 2, maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, linkThickness);
                    break;
                }
                case zlIIR::FilterType::notch:
                case zlIIR::FilterType::lowPass:
                case zlIIR::FilterType::highPass:
                case zlIIR::FilterType::bandPass: {
                    const auto x1 = freqToX(baseFreq.load(), bound);
                    const auto y1 = dbToY(centeredDB.load(), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(0), maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, linkThickness);
                    break;
                }
            }
        }
    }

    void SinglePanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        xx.store(bound.getX());
        yy.store(bound.getY());
        width.store(bound.getWidth());
        height.store(bound.getHeight());
        sidePanel.setBounds(getLocalBounds());
        toRepaint.store(true);
    }

    bool SinglePanel::willRepaint() const {
        return toRepaint.load();
    }

    bool SinglePanel::checkRepaint() {
        if (baseF.getMagOutdated() || targetF.getMagOutdated()) {
            return true;
        } else if (toRepaint.exchange(false)) {
            return true;
        }
        return sidePanel.checkRepaint();
    }

    void SinglePanel::drawCurve(juce::Path &path,
                                const std::array<double, zlIIR::frequencies.size()> &dBs,
                                juce::Rectangle<float> bound,
                                const bool reverse,
                                const bool startPath) {
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
            if (parameterID.startsWith(zlState::active::ID)) {
                actived.store(static_cast<bool>(newValue));
                baseFreq.store(10.0);
                baseGain.store(0.0);
            } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
                dynON.store(static_cast<bool>(newValue));
            }
        }
        toRepaint.store(true);
    }

    void SinglePanel::run() {
        juce::ScopedNoDenormals noDenormals;
        const juce::Rectangle<float> bound{xx.load(), yy.load(), width.load(), height.load()};
        // draw curve
        baseFreq.store(static_cast<double>(baseF.getFreq()));
        baseGain.store(static_cast<double>(baseF.getGain()));
        {
            baseF.updateDBs();
            curvePath.clear();
            if (actived.load()) {
                drawCurve(curvePath, baseF.getDBs(), bound);
                centeredDB.store(static_cast<float>(baseF.getDB(baseFreq.load())));
            } else {
                centeredDB.store(0.f);
            }
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::realtime> pathLock(recentCurvePath);
            *pathLock = curvePath;
        }
        // draw shadow
        {
            shadowPath.clear();
            if (actived.load()) {
                drawCurve(shadowPath, baseF.getDBs(), bound);
            }
            if (actived.load() && selected.load()) {
                switch (baseF.getFilterType()) {
                    case zlIIR::FilterType::peak:
                    case zlIIR::FilterType::lowShelf:
                    case zlIIR::FilterType::highShelf:
                    case zlIIR::FilterType::notch:
                    case zlIIR::FilterType::bandShelf:
                    case zlIIR::FilterType::tiltShelf: {
                        shadowPath.lineTo(bound.getRight(), bound.getCentreY());
                        shadowPath.lineTo(bound.getX(), bound.getCentreY());
                        shadowPath.closeSubPath();
                        break;
                    }
                    case zlIIR::FilterType::lowPass:
                    case zlIIR::FilterType::highPass:
                    case zlIIR::FilterType::bandPass: {
                        shadowPath.lineTo(bound.getBottomRight());
                        shadowPath.lineTo(bound.getBottomLeft());
                        shadowPath.closeSubPath();
                        break;
                    }
                }
            }
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::realtime> pathLock(recentShadowPath);
            *pathLock = shadowPath;
        }
        // draw dynamic shadow
        {
            dynPath.clear();
            if (dynON.load() && actived.load()) {
                drawCurve(dynPath, baseF.getDBs(), bound);
                targetF.updateDBs();
                drawCurve(dynPath, targetF.getDBs(), bound, true, false);
                dynPath.closeSubPath();
            }
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::realtime> pathLock(recentDynPath);
            *pathLock = dynPath;
        }
    }
} // zlPanel
