// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class MatchRunner final : private juce::Thread,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater,
                              private juce::ValueTree::Listener {
    public:
        explicit MatchRunner(PluginProcessor &p, zlgui::UIBase &base,
                             std::array<std::atomic<float>, 251> &atomic_diffs,
                             zlgui::CompactLinearSlider &num_band_slider);

        ~MatchRunner() override;

        void start();

        void setMode(const size_t x) {
            mode_.store(x);
        }

        void setNumBand(const size_t x) {
            num_band_.store(x);
        }

        void update() {
            triggerAsyncUpdate();
        }

        void setMaximumDB(const float x) { maximum_db_.store(x); }

    private:
        zlgui::UIBase &ui_base_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zldsp::eq_match::EqMatchOptimizer<16> optimizer_;
        std::array<std::atomic<float>, 251> &atomic_diffs_ref_;
        zlgui::CompactLinearSlider &num_band_slider_ref_;
        std::array<double, 251> diffs_{};
        std::atomic<bool> to_calculate_num_band_{false};
        std::atomic<size_t> mode_{1}, num_band_{8};
        size_t est_num_band_{16};
        std::array<zldsp::filter::Empty<double>, 16> filters_;
        juce::CriticalSection critical_section_;
        std::atomic<float> low_cut_p_{0.f}, high_cut_p_{1.f}, maximum_db_{12.f};

        void run() override;

        void handleAsyncUpdate() override;

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;

        void savePara(const std::string &id, const float x) const {
            const auto para = parameters_ref_.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }

        void loadDiffs();

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        static constexpr double mseRelThreshold = 1.f / 30.f;
    };
} // zlpanel
