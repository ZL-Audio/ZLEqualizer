// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef MATCH_RUNNER_HPP
#define MATCH_RUNNER_HPP

#include "../../../PluginProcessor.hpp"

namespace zlPanel {
    class MatchRunner final : private juce::Thread,
                              private juce::AsyncUpdater {
    public:
        explicit MatchRunner(PluginProcessor &p, std::array<std::atomic<float>, 251> &atomicDiffs);

        void start();

        void setMode(const size_t x) {
            mode.store(x);
        }

        void setNumBand(const size_t x) {
            numBand.store(x);
        }

        void update() {
            triggerAsyncUpdate();
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlEqMatch::EqMatchOptimizer<16> optimizer;
        std::array<std::atomic<float>, 251> &atomicDiffsRef;
        std::array<double, 251> diffs{};
        std::atomic<bool> isReady{false}, isRunning{false};
        std::atomic<size_t> mode{1}, numBand{8};

        void run() override;

        void handleAsyncUpdate() override;

        void savePara(const std::string &id, const float x) const {
            const auto para = parametersRef.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }

        void loadDiffs();
    };
} // zlPanel

#endif //MATCH_RUNNER_HPP
