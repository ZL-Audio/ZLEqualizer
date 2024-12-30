// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_runner.hpp"

namespace zlPanel {
    MatchRunner::MatchRunner(PluginProcessor &p, zlInterface::UIBase &base,
                             std::array<std::atomic<float>, 251> &atomicDiffs,
                             zlInterface::CompactLinearSlider &numBandSlider)
        : Thread("match_runner"), uiBase(base),
          parametersRef(p.parameters), parametersNARef(p.parametersNA),
          atomicDiffsRef(atomicDiffs),
          slider(numBandSlider) {
        std::fill(diffs.begin(), diffs.end(), 0.);
        uiBase.getValueTree().addListener(this);
        addListener(&optimizer);
    }

    MatchRunner::~MatchRunner() {
        stopThread(-1);
        removeListener(&optimizer);
        uiBase.getValueTree().removeListener(this);
    }

    void MatchRunner::start() {
        startThread(Priority::low);
    }

    void MatchRunner::run() {
        const auto startIdx = static_cast<size_t>(lowCutP.load() * static_cast<float>(diffs.size()));
        const auto endIdx = static_cast<size_t>(highCutP.load() * static_cast<float>(diffs.size()));
        if (mode.load() == 0) {
            loadDiffs();
            optimizer.setDiffs(&diffs[0], diffs.size());
            optimizer.runDeterministic(startIdx, endIdx);
        } else if (mode.load() == 1) {
            loadDiffs();
            optimizer.setDiffs(&diffs[0], diffs.size());
            optimizer.runStochastic(startIdx, endIdx);
        } else {
            loadDiffs();
            optimizer.setDiffs(&diffs[0], diffs.size());
            optimizer.runStochasticPlus({2, 4, 6}, startIdx, endIdx);
        }
        if (threadShouldExit()) {
            return;
        } {
            juce::ScopedLock lock(criticalSection);
            const auto &filters = optimizer.getSol();
            for (size_t i = 0; i < mFilters.size(); i++) {
                mFilters[i].setFilterType(filters[i].getFilterType());
                mFilters[i].setOrder(filters[i].getOrder());
                mFilters[i].setFreq(filters[i].getFreq());
                mFilters[i].setGain(filters[i].getGain());
                mFilters[i].setQ(filters[i].getQ());
            }
            estNumBand = 16;
            const auto &mse = optimizer.getMSE();
            for (size_t i = 0; i < mFilters.size(); i++) {
                if (mse[i] < mseThreshold) {
                    estNumBand = i + 1;
                    break;
                }
            }
            toCalculateNumBand.store(true);
        }
        triggerAsyncUpdate();
    }

    void MatchRunner::handleAsyncUpdate() {
        juce::ScopedLock lock(criticalSection);
        size_t currentNumBand = numBand.load();
        if (toCalculateNumBand.exchange(false)) {
            currentNumBand = estNumBand;
            slider.getSlider().setValue(static_cast<double>(currentNumBand), juce::dontSendNotification);
            slider.getSlider().setDoubleClickReturnValue(true, static_cast<double>(currentNumBand));
            slider.updateDisplayValue();
            numBand.store(currentNumBand);
            uiBase.setProperty(zlInterface::settingIdx::matchFitRunning, false);
        }
        for (size_t i = 0; i < currentNumBand; i++) {
            const auto &filter = mFilters[i];
            savePara(zlDSP::appendSuffix(zlDSP::bypass::ID, i), 0.f);
            savePara(zlDSP::appendSuffix(zlDSP::dynamicON::ID, i), 0.f);
            savePara(zlDSP::appendSuffix(zlDSP::fType::ID, i),
                     zlDSP::fType::convertTo01(filter.getFilterType()));
            savePara(zlDSP::appendSuffix(zlDSP::slope::ID, i),
                     zlDSP::slope::convertTo01(zlDSP::slope::convertToIdx(filter.getOrder())));
            savePara(zlDSP::appendSuffix(zlDSP::freq::ID, i),
                     zlDSP::freq::convertTo01(static_cast<float>(filter.getFreq())));
            savePara(zlDSP::appendSuffix(zlDSP::gain::ID, i),
                     zlDSP::gain::convertTo01(static_cast<float>(filter.getGain())));
            savePara(zlDSP::appendSuffix(zlDSP::Q::ID, i),
                     zlDSP::Q::convertTo01(static_cast<float>(filter.getQ())));
        }
        for (size_t i = currentNumBand; i < mFilters.size(); i++) {
            const auto para = parametersNARef.getParameter(zlState::appendSuffix(zlState::active::ID, i));
            para->beginChangeGesture();
            para->setValueNotifyingHost(0.f);
            para->endChangeGesture();
        }
    }

    void MatchRunner::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                               const juce::Identifier &property) {
        juce::ignoreUnused(treeWhosePropertyHasChanged);
        if (property == zlInterface::identifiers[static_cast<size_t>(zlInterface::settingIdx::matchLowCut)]) {
            lowCutP.store(static_cast<float>(uiBase.getProperty(zlInterface::settingIdx::matchLowCut)));
        } else if (property == zlInterface::identifiers[static_cast<size_t>(zlInterface::settingIdx::matchHighCut)]) {
            highCutP.store(static_cast<float>(uiBase.getProperty(zlInterface::settingIdx::matchHighCut)));
        }
    }

    void MatchRunner::loadDiffs() {
        for (size_t i = 0; i < diffs.size(); i++) {
            diffs[i] = static_cast<double>(atomicDiffsRef[i].load());
        }
    }
} // zlPanel
