// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlPanel {
    SinglePanel::SinglePanel(const size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base,
                             zlDSP::Controller<double> &controller,
                             zlFilter::Ideal<double, 16> &baseFilter,
                             zlFilter::Ideal<double, 16> &targetFilter,
                             zlFilter::Ideal<double, 16> &mainFilter)
        : idx(bandIdx), parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base), controllerRef(controller),
          resetAttach(bandIdx, parameters, parametersNA),
          baseF(baseFilter), targetF(targetFilter), mainF(mainFilter) {
        curvePath.preallocateSpace(static_cast<int>(zlFilter::frequencies.size() * 3 + 12));
        shadowPath.preallocateSpace(static_cast<int>(zlFilter::frequencies.size() * 3 + 12));
        dynPath.preallocateSpace(static_cast<int>(zlFilter::frequencies.size() * 6 + 12));

        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        juce::ignoreUnused(controllerRef);

        skipRepaint.store(true);
        parameterChanged(zlState::selectedBandIdx::ID,
                         parametersNARef.getRawParameterValue(zlState::selectedBandIdx::ID)->load());
        parameterChanged(zlState::active::ID + suffix,
                         parametersNARef.getRawParameterValue(zlState::active::ID + suffix)->load());
        parametersNARef.addParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.addParameterListener(zlState::active::ID + suffix, this);

        for (auto &id: changeIDs) {
            const auto paraId = id + suffix;
            parameterChanged(paraId, parametersRef.getRawParameterValue(paraId)->load());
            parametersRef.addParameterListener(paraId, this);
        }
        for (auto &id: paraIDs) {
            const auto paraId = id + suffix;
            parameterChanged(paraId, parametersRef.getRawParameterValue(paraId)->load());
            parametersRef.addParameterListener(paraId, this);
        }

        setInterceptsMouseClicks(false, false);
        skipRepaint.store(false);
        lookAndFeelChanged();
    }

    SinglePanel::~SinglePanel() {
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        for (auto &id: changeIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        for (auto &id: paraIDs) {
            parametersRef.removeParameterListener(id + suffix, this);
        }
        parametersNARef.removeParameterListener(zlState::selectedBandIdx::ID, this);
        parametersNARef.removeParameterListener(zlState::active::ID + suffix, this);
    }

    void SinglePanel::paint(juce::Graphics &g) {
        if (!active.load()) {
            return;
        }
        // draw curve
        {
            g.setColour(colour);
            const juce::GenericScopedTryLock lock(curveLock);
            if (lock.isLocked()) {
                if (uiBase.getIsRenderingHardware()) {
                    g.strokePath(recentCurvePath, juce::PathStrokeType(curveThickness.load(),
                                                                       juce::PathStrokeType::curved,
                                                                       juce::PathStrokeType::rounded));
                } else {
                    g.fillPath(recentCurvePath);
                }
            }
        }
        // draw shadow
        if (selected.load()) {
            g.setColour(colour.withMultipliedAlpha(0.125f));
            const juce::GenericScopedTryLock lock(shadowLock);
            if (lock.isLocked()) {
                g.fillPath(recentShadowPath);
            }
        }
        // draw dynamic shadow
        if (dynON.load()) {
            if (selected.load()) {
                g.setColour(colour.withMultipliedAlpha(0.25f));
            } else {
                g.setColour(colour.withMultipliedAlpha(0.125f));
            }
            const juce::GenericScopedTryLock lock(dynLock);
            if (lock.isLocked()) {
                g.fillPath(recentDynPath);
            }
        }
        // draw the line between the curve and the button
        {
            const auto linkThickness = uiBase.getFontSize() * 0.065f * uiBase.getSingleCurveThickness();
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
            g.setColour(colour);
            switch (baseF.getFilterType()) {
                case zlFilter::FilterType::peak:
                case zlFilter::FilterType::bandShelf: {
                    const auto x1 = freqToX(baseFreq.load(), bound);
                    const auto y1 = dbToY(centeredDB.load(), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(baseGain.load()), maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, linkThickness);
                    break;
                }
                case zlFilter::FilterType::lowShelf:
                case zlFilter::FilterType::highShelf:
                case zlFilter::FilterType::tiltShelf: {
                    const auto x1 = freqToX(baseFreq.load(), bound);
                    const auto y1 = dbToY(centeredDB.load(), maximumDB.load(), bound);
                    const auto y2 = dbToY(static_cast<float>(baseGain.load()) / 2, maximumDB.load(), bound);
                    g.drawLine(x1, y1, x1, y2, linkThickness);
                    break;
                }
                case zlFilter::FilterType::notch:
                case zlFilter::FilterType::lowPass:
                case zlFilter::FilterType::highPass:
                case zlFilter::FilterType::bandPass: {
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
        auto bound = getLocalBounds().toFloat();
        atomicBottomLeft.store(bound.getBottomLeft());
        atomicBottomRight.store(bound.getBottomRight());
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        atomicBound.store(bound);
        toRepaint.store(true);
        handleAsyncUpdate();
    }

    bool SinglePanel::willRepaint() const {
        return toRepaint.load();
    }

    bool SinglePanel::checkRepaint() {
        if (!active.load()) return false;
        if (baseF.getMagOutdated() || (dynON.load() && targetF.getMagOutdated())) {
            return true;
        } else if (toRepaint.exchange(false)) {
            return true;
        }
        return false;
    }

    void SinglePanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlState::selectedBandIdx::ID) {
            selected.store(static_cast<size_t>(newValue) == idx);
            if (!uiBase.getIsRenderingHardware()) triggerAsyncUpdate();
        } else {
            if (parameterID.startsWith(zlState::active::ID)) {
                active.store(newValue > .5f);
                baseFreq.store(10.0);
                baseGain.store(0.0);
            } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
                dynON.store(newValue > .5f);
            } else if (parameterID.startsWith(zlDSP::fType::ID)) {
                baseF.setFilterType(static_cast<zlFilter::FilterType>(newValue));
                mainF.setFilterType(static_cast<zlFilter::FilterType>(newValue));
                targetF.setFilterType(static_cast<zlFilter::FilterType>(newValue));
            } else if (parameterID.startsWith(zlDSP::slope::ID)) {
                const auto order = zlDSP::slope::orderArray[static_cast<size_t>(newValue)];
                baseF.setOrder(order);
                mainF.setOrder(order);
                targetF.setOrder(order);
            } else if (parameterID.startsWith(zlDSP::freq::ID)) {
                baseF.setFreq(static_cast<double>(newValue));
                mainF.setFreq(static_cast<double>(newValue));
                targetF.setFreq(static_cast<double>(newValue));
            } else if (parameterID.startsWith(zlDSP::gain::ID)) {
                currentBaseGain.store(static_cast<double>(newValue));
                baseF.setGain(static_cast<double>(zlDSP::gain::range.snapToLegalValue(
                    newValue * static_cast<float>(scale.load()))));
            } else if (parameterID.startsWith(zlDSP::Q::ID)) {
                baseF.setQ(static_cast<double>(newValue));
            } else if (parameterID.startsWith(zlDSP::targetGain::ID)) {
                currentTargetGain.store(static_cast<double>(newValue));
                targetF.setGain(static_cast<double>(zlDSP::targetGain::range.snapToLegalValue(
                    newValue * static_cast<float>(scale.load()))));
            } else if (parameterID.startsWith(zlDSP::targetQ::ID)) {
                targetF.setQ(static_cast<double>(newValue));
            }
        }
        toRepaint.store(true);
    }

    void SinglePanel::run(const float physicalPixelScaleFactor) {
        juce::ScopedNoDenormals noDenormals;
        const auto bound = atomicBound.load();
        const auto bottomLeft = atomicBottomLeft.load();
        const auto bottomRight = atomicBottomRight.load();
        const auto maxDB = maximumDB.load();
        // draw curve
        baseFreq.store(static_cast<double>(baseF.getFreq()));
        baseGain.store(static_cast<double>(baseF.getGain())); {
            baseF.updateMagnitude(ws);
            curvePath.clear();
            if (active.load()) {
                drawCurve(curvePath, baseF.getDBs(), maxDB, bound);
                centeredDB.store(static_cast<float>(baseF.getDB(0.0001308996938995747 * baseFreq.load())));
            } else {
                centeredDB.store(0.f);
            }
            if (uiBase.getIsRenderingHardware()) {
                juce::GenericScopedLock lock(curveLock);
                recentCurvePath = curvePath;
            } else {
                juce::PathStrokeType stroke(curveThickness.load(), juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded);
                stroke.createStrokedPath(strokePath, curvePath, {}, physicalPixelScaleFactor);
                juce::GenericScopedLock lock(curveLock);
                recentCurvePath = strokePath;
            }
        }
        // draw shadow
        {
            shadowPath.clear();
            if (active.load()) {
                drawCurve(shadowPath, baseF.getDBs(), maxDB, bound);
            }
            if (active.load() && selected.load()) {
                switch (baseF.getFilterType()) {
                    case zlFilter::FilterType::peak:
                    case zlFilter::FilterType::lowShelf:
                    case zlFilter::FilterType::highShelf:
                    case zlFilter::FilterType::notch:
                    case zlFilter::FilterType::bandShelf:
                    case zlFilter::FilterType::tiltShelf: {
                        shadowPath.lineTo(bound.getRight(), bound.getCentreY());
                        shadowPath.lineTo(bound.getX(), bound.getCentreY());
                        shadowPath.closeSubPath();
                        break;
                    }
                    case zlFilter::FilterType::lowPass:
                    case zlFilter::FilterType::highPass:
                    case zlFilter::FilterType::bandPass: {
                        shadowPath.lineTo(bottomRight);
                        shadowPath.lineTo(bottomLeft);
                        shadowPath.closeSubPath();
                        break;
                    }
                }
            }
            juce::GenericScopedLock lock(shadowLock);
            recentShadowPath = shadowPath;
        }
        // draw dynamic shadow
        {
            dynPath.clear();
            if (dynON.load() && active.load()) {
                drawCurve(dynPath, baseF.getDBs(), maxDB, bound);
                targetF.updateMagnitude(ws);
                drawCurve(dynPath, targetF.getDBs(), maxDB, bound, true, false);
                dynPath.closeSubPath();
            }
            juce::GenericScopedLock lock(dynLock);
            recentDynPath = dynPath;
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        colour = uiBase.getColorMap1(idx);
        handleAsyncUpdate();
    }

    void SinglePanel::handleAsyncUpdate() {
        auto thickness = selected.load() ? uiBase.getFontSize() * 0.15f : uiBase.getFontSize() * 0.075f;
        thickness *= uiBase.getSingleCurveThickness();
        curveThickness.store(thickness);
    }
} // zlPanel
