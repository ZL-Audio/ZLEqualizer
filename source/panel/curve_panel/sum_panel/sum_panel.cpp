// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlPanel {
    SumPanel::SumPanel(juce::AudioProcessorValueTreeState &parameters,
                       zlInterface::UIBase &base,
                       zlDSP::Controller<double> &controller,
                       std::array<zlFilter::Ideal<double, 16>, 16> &baseFilters,
                       std::array<zlFilter::Ideal<double, 16>, 16> &mainFilters)
        : parametersRef(parameters),
          uiBase(base), c(controller),
          mBaseFilters(baseFilters), mMainFilters(mainFilters) {
        dBs.resize(ws.size());
        for (auto &path: paths) {
            path.preallocateSpace(static_cast<int>(zlFilter::frequencies.size() * 3));
        }
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            for (const auto &idx: changeIDs) {
                parametersRef.addParameterListener(zlDSP::appendSuffix(idx, i), this);
            }
        }
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
            if (!c.getFilter(i).getBypass()) {
                useLRMS[idx] = true;
            }
        }
        for (size_t j = 0; j < useLRMS.size(); ++j) {
            if (!useLRMS[j]) { continue; }
            g.setColour(uiBase.getColorMap2(j));
            const juce::GenericScopedTryLock lock(pathLocks[j]);
            if (lock.isLocked()) {
                g.strokePath(recentPaths[j], juce::PathStrokeType(
                                 uiBase.getFontSize() * 0.2f * uiBase.getSumCurveThickness(),
                                 juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    bool SumPanel::checkRepaint() {
        for (size_t i = 0; i < zlState::bandNUM; ++i) {
            if (c.getFilter(i).getMainFilter().getMagOutdated()) {
                return true;
            }
        }
        if (toRepaint.exchange(false)) {
            return true;
        }
        return false;
    }

    void SumPanel::run() {
        juce::ScopedNoDenormals noDenormals;
        std::array<bool, 5> useLRMS{false, false, false, false, false};
        constexpr std::array<zlDSP::lrType::lrTypes, 5> lrTypes{
            zlDSP::lrType::stereo,
            zlDSP::lrType::left, zlDSP::lrType::right,
            zlDSP::lrType::mid, zlDSP::lrType::side
        };
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const auto idx = static_cast<size_t>(c.getFilterLRs(i));
            if (!c.getFilter(i).getBypass()) {
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
                auto &filter{c.getFilter(i)};
                if (c.getFilterLRs(i) == lrTypes[j] && !filter.getBypass()) {
                    if (filter.getDynamicON()) {
                        mMainFilters[i].setGain(filter.getMainFilter().getGain());
                        mMainFilters[i].setQ(filter.getMainFilter().getQ());
                        mMainFilters[i].updateMagnidue(ws);
                        mMainFilters[i].addDBs(dBs);
                    } else {
                        mBaseFilters[i].addDBs(dBs);
                    }
                }
            }

            const juce::Rectangle<float> bound{
                xx.load(), yy.load() + uiBase.getFontSize(),
                width.load(), height.load() - 2 * uiBase.getFontSize()
            };

            drawCurve(paths[j], dBs, maximumDB.load(), bound, false, true);
        }
        for (size_t j = 0; j < useLRMS.size(); ++j) {
            juce::GenericScopedLock lock(pathLocks[j]);
            recentPaths[j] = paths[j];
        }
    }

    void SumPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        toRepaint.store(true);
    }

    void SumPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        xx.store(bound.getX());
        yy.store(bound.getY());
        width.store(bound.getWidth());
        height.store(bound.getHeight());
        toRepaint.store(true);
    }
} // zlPanel
