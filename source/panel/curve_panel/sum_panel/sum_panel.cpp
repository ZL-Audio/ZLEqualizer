// Copyright (C) 2023 - zsliu98
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
                       zlDSP::Controller<double> &controller)
        : parametersRef(parameters),
          uiBase(base), c(controller) {
        for (auto &path: paths) {
            path.preallocateSpace(zlIIR::frequencies.size() * 3);
        }
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            for (const auto &idx: changeIDs) {
                parametersRef.addParameterListener(zlDSP::appendSuffix(idx, i), this);
            }
        }
        juce::ignoreUnused(parametersRef);
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
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::nonRealtime> pathLock(recentPaths[j]);
            g.strokePath(*pathLock, juce::PathStrokeType(uiBase.getFontSize() * 0.2f * uiBase.getSumCurveThickness(),
                                                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
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
            zlDSP::lrType::stereo, zlDSP::lrType::left, zlDSP::lrType::right, zlDSP::lrType::mid,
            zlDSP::lrType::side
        };
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const auto idx = static_cast<size_t>(c.getFilterLRs(i));
            if (!c.getFilter(i).getBypass()) {
                useLRMS[idx] = true;
            }
        }

        for (size_t j = 0; j < useLRMS.size(); ++j) {
            if (!useLRMS[j]) { continue; }

            c.updateDBs(lrTypes[j]);
            const auto &dBs = c.getDBs();

            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());

            const auto maxDB = maximumDB.load();
            auto y0 = 0.f;
            paths[j].clear();
            for (size_t i = 0; i < zlIIR::frequencies.size(); ++i) {
                const auto x = static_cast<float>(i) / static_cast<float>(zlIIR::frequencies.size() - 1) * bound.
                               getWidth();
                const auto y = static_cast<float>(-dBs[i]) / maxDB * bound.getHeight() * 0.5f + bound.getCentreY();
                if (i == 0) {
                    paths[j].startNewSubPath(x, y);
                    y0 = y;
                } else if (std::abs(y - y0) >= 0.125f || i == zlIIR::frequencies.size() - 1) {
                    paths[j].lineTo(x, y);
                    y0 = y;
                }
            }
        }
        for (size_t j = 0; j < useLRMS.size(); ++j) {
            farbot::RealtimeObject<
                juce::Path,
                farbot::RealtimeObjectOptions::realtimeMutatable>::ScopedAccess<
                farbot::ThreadType::realtime> pathLock(recentPaths[j]);
            *pathLock = paths[j];
        }
        triggerAsyncUpdate();
    }

    void SumPanel::handleAsyncUpdate() {
        repaint();
    }

    void SumPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID, newValue);
        toRepaint.store(true);
    }

    void SumPanel::resized() {
        toRepaint.store(true);
    }
} // zlPanel
