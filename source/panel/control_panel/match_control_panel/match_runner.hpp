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
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class MatchRunner final : private juce::Thread,
                              private juce::AsyncUpdater,
                              private juce::ValueTree::Listener {
    public:
        explicit MatchRunner(PluginProcessor &p, zlInterface::UIBase &base,
                             std::array<std::atomic<float>, 251> &atomicDiffs,
                             zlInterface::CompactLinearSlider &numBandSlider);

        ~MatchRunner() override;

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
        zlInterface::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlEqMatch::EqMatchOptimizer<16> optimizer;
        std::array<std::atomic<float>, 251> &atomicDiffsRef;
        zlInterface::CompactLinearSlider &slider;
        std::array<double, 251> diffs{};
        std::atomic<bool> toCalculateNumBand{false};
        std::atomic<size_t> mode{1}, numBand{8};
        size_t estNumBand{16};
        std::array<zlFilter::Empty<double>, 16> mFilters;
        juce::CriticalSection criticalSection;
        std::atomic<float> lowCutP{0.f}, highCutP{1.f};

        void run() override;

        void handleAsyncUpdate() override;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;

        void savePara(const std::string &id, const float x) const {
            const auto para = parametersRef.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }

        void loadDiffs();

        static constexpr double mseThreshold = .5;
    };
} // zlPanel

#endif //MATCH_RUNNER_HPP
