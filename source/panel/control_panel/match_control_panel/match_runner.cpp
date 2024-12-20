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
    MatchRunner::MatchRunner(PluginProcessor &p,
                             std::array<std::atomic<float>, 251> &atomicDiffs,
                             zlInterface::CompactLinearSlider &numBandSlider)
        : Thread("match_runner"),
          parametersRef(p.parameters), parametersNARef(p.parametersNA),
          atomicDiffsRef(atomicDiffs),
          slider(numBandSlider) {
        std::fill(diffs.begin(), diffs.end(), 0.);
    }

    void MatchRunner::start() {
        startThread(Priority::low);
    }

    void MatchRunner::run() {
        if (mode.load() == 0) {
            loadDiffs();
            optimizer.setDiffs(&diffs[0], diffs.size());
            optimizer.runDeterministic();
        } else {
            loadDiffs();
            optimizer.setDiffs(&diffs[0], diffs.size());
            optimizer.runStochastic();
        }
        juce::ScopedLock lock(criticalSection);
        const auto &filters = optimizer.getSol();
        for (size_t i = 0; i < mFilters.size(); i++) {
            mFilters[i].setFilterType(filters[i].getFilterType());
            mFilters[i].setFreq(filters[i].getFreq());
            mFilters[i].setGain(filters[i].getGain());
            mFilters[i].setQ(filters[i].getQ());
        }
        toCalculateNumBand.store(true);
        triggerAsyncUpdate();
    }

    void MatchRunner::handleAsyncUpdate() {
        juce::ScopedLock lock(criticalSection);
        size_t currentNumBand = numBand.load();
        if (toCalculateNumBand.exchange(false)) {
            currentNumBand = 16;
            for (size_t i = 0; i < 16; i++) {
                if (calculateEffect(mFilters[16 - i]) < effectThreshold) {
                    currentNumBand -= 1;
                } else {
                    break;
                }
            }
            currentNumBand = std::max(currentNumBand, static_cast<size_t>(1));
            slider.getSlider().setValue(static_cast<double>(currentNumBand), juce::dontSendNotification);
            slider.updateDisplayValue();
            numBand.store(currentNumBand);
        }
        for (size_t i = 0; i < currentNumBand; i++) {
            const auto &filter = mFilters[i];
            savePara(zlDSP::appendSuffix(zlDSP::bypass::ID, i), 0.f);
            savePara(zlDSP::appendSuffix(zlDSP::dynamicON::ID, i), 0.f);
            savePara(zlDSP::appendSuffix(zlDSP::fType::ID, i),
                     zlDSP::fType::convertTo01(filter.getFilterType()));
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

    void MatchRunner::loadDiffs() {
        for (size_t i = 0; i < diffs.size(); i++) {
            diffs[i] = static_cast<double>(atomicDiffsRef[i].load());
        }
    }

    double MatchRunner::calculateEffect(const zlFilter::Empty<double> &filter) {
        const auto fType = filter.getFilterType();
        const auto freq = filter.getFreq(), gain = filter.getGain(), q = filter.getQ();
        switch (fType) {
            case zlFilter::peak: {
                return std::abs(gain) * std::asinh(0.5 / q) * 2.0 / std::log(2.0);
            }
            case zlFilter::lowShelf: {
                return std::abs(gain) * (std::log2(freq / 10.0) + 0.1);
            }
            case zlFilter::highShelf: {
                return std::abs(gain) * (std::log2(22000.0 / freq) + 0.1);
            }
            case zlFilter::lowPass:
            case zlFilter::highPass:
            case zlFilter::notch:
            case zlFilter::bandPass:
            case zlFilter::tiltShelf:
            case zlFilter::bandShelf:
            default: {
                return 0.;
            }
        }
    }
} // zlPanel
