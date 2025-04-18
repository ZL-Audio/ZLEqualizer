// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlPanel {
    SumPanel::SumPanel(juce::AudioProcessorValueTreeState &parameters,
                       zlInterface::UIBase &base,
                       zlDSP::Controller<double> &controller,
                       std::array<zlFilter::Ideal<double, 16>, 16> &baseFilters,
                       std::array<zlFilter::Ideal<double, 16>, 16> &mainFilters)
        : parametersRef(parameters),
          uiBase(base), c(controller),
          mMainFilters(mainFilters) {
        juce::ignoreUnused(baseFilters);
        dBs.resize(ws.size());
        for (auto &path: paths) {
            path.preallocateSpace(static_cast<int>(zlFilter::frequencies.size() * 3));
        }
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            for (const auto &idx: changeIDs) {
                const auto paraID = zlDSP::appendSuffix(idx, i);
                parameterChanged(paraID, parametersRef.getRawParameterValue(paraID)->load());
                parametersRef.addParameterListener(paraID, this);
            }
        }
        lookAndFeelChanged();
    }

    SumPanel::~SumPanel() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            for (const auto &idx: changeIDs) {
                parametersRef.removeParameterListener(zlDSP::appendSuffix(idx, i), this);
            }
        }
    }

    void SumPanel::paint(juce::Graphics &g) {
        std::array<bool, 5> useLRMS{false, false, false, false, false};
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const auto idx = static_cast<size_t>(c.getFilterLRs(i));
            if (!c.getBypass(i)) {
                useLRMS[idx] = true;
            }
        }
        if (uiBase.getIsRenderingHardware()) {
            const auto currentThickness = curveThickness.load();
            for (size_t j = 0; j < useLRMS.size(); ++j) {
                if (!useLRMS[j]) { continue; }
                g.setColour(colours[j]);
                const juce::GenericScopedTryLock lock(pathLocks[j]);
                if (lock.isLocked()) {
                    g.strokePath(recentPaths[j], juce::PathStrokeType(
                                     currentThickness,
                                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
            }
        } else {
            for (size_t j = 0; j < useLRMS.size(); ++j) {
                if (!useLRMS[j]) { continue; }
                g.setColour(colours[j]);
                const juce::GenericScopedTryLock lock(pathLocks[j]);
                if (lock.isLocked()) {
                    g.fillPath(recentPaths[j]);
                }
            }
        }
    }

    bool SumPanel::checkRepaint() {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            if (mMainFilters[i].getMagOutdated()) {
                return true;
            }
        }
        if (toRepaint.exchange(false)) {
            return true;
        }
        return false;
    }

    void SumPanel::run(const float physicalPixelScaleFactor) {
        juce::ScopedNoDenormals noDenormals;
        const auto isHardware = uiBase.getIsRenderingHardware();
        std::array<bool, 5> useLRMS{false, false, false, false, false};
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const auto idx = static_cast<size_t>(lrTypes[i].load());
            if (!isBypassed[i].load()) {
                useLRMS[idx] = true;
            }
        }

        for (size_t j = 0; j < useLRMS.size(); ++j) {
            paths[j].clear();
            if (!useLRMS[j]) {
                continue;
            }

            std::fill(dBs.begin(), dBs.end(), 0.0);
            for (size_t i = 0; i < zlState::bandNUM; i++) {
                auto &filter{c.getMainIdealFilter(i)};
                if (lrTypes[i].load() == static_cast<zlDSP::lrType::lrTypes>(j) && !isBypassed[i].load()) {
                    mMainFilters[i].setGain(filter.getGain());
                    mMainFilters[i].setQ(filter.getQ());
                    mMainFilters[i].updateMagnitude(ws);
                    mMainFilters[i].addDBs(dBs);
                }
            }

            drawCurve(paths[j], dBs, maximumDB.load(), atomicBound.load(), false, true);
            if (!isHardware) {
                juce::PathStrokeType stroke{
                    curveThickness.load(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded
                };
                stroke.createStrokedPath(strokePaths[j], paths[j], {}, physicalPixelScaleFactor);
            }
        }
        if (isHardware) {
            for (size_t j = 0; j < useLRMS.size(); ++j) {
                juce::GenericScopedLock lock(pathLocks[j]);
                recentPaths[j] = paths[j];
            }
        } else {
            for (size_t j = 0; j < useLRMS.size(); ++j) {
                juce::GenericScopedLock lock(pathLocks[j]);
                recentPaths[j] = strokePaths[j];
            }
        }
    }

    void SumPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (parameterID.startsWith(zlDSP::bypass::ID)) {
            isBypassed[idx].store(newValue > .5f);
        } else if (parameterID.startsWith(zlDSP::lrType::ID)) {
            lrTypes[idx].store(static_cast<zlDSP::lrType::lrTypes>(newValue));
        }
        toRepaint.store(true);
    }

    void SumPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomicBound.store({
            bound.getX(), bound.getY() + uiBase.getFontSize(),
            bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize()
        });
        toRepaint.store(true);
        updateCurveThickness();
    }

    void SumPanel::lookAndFeelChanged() {
        for (size_t j = 0; j < colours.size(); ++j) {
            colours[j] = uiBase.getColorMap2(j);
        }
        updateCurveThickness();
    }

    void SumPanel::updateCurveThickness() {
        curveThickness.store(uiBase.getFontSize() * 0.2f * uiBase.getSumCurveThickness());
    }
} // zlPanel
