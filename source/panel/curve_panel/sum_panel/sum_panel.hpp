// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"

namespace zlpanel {
    class SumPanel final : public juce::Component,
                           private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SumPanel(juce::AudioProcessorValueTreeState &parameters,
                          zlgui::UIBase &base,
                          zlp::Controller<double> &controller,
                          std::array<zldsp::filter::Ideal<double, 16>, 16> &base_filters,
                          std::array<zldsp::filter::Ideal<double, 16>, 16> &main_filters);

        ~SumPanel() override;

        void paint(juce::Graphics &g) override;

        void setMaximumDB(const float x) {
            maximum_db_.store(x);
            to_repaint_.store(true);
        }

        bool checkRepaint();

        void resized() override;

        void run(float physicalPixelScaleFactor);

        void lookAndFeelChanged() override;

    private:
        std::array<juce::Path, 5> paths_, recent_paths_, stroke_paths_;
        std::array<juce::SpinLock, 5> path_locks_;
        std::array<juce::Colour, 5> colours_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;
        std::array<zldsp::filter::Ideal<double, 16>, zlstate::kBandNUM> &main_filters_;
        std::atomic<float> maximum_db_;
        std::vector<double> dbs_{};
        AtomicBound<float> atomic_bound_;
        std::atomic<float> curve_thickness_{0.f};

        static constexpr std::array kChangeIDs{
            zlp::bypass::ID, zlp::lrType::ID
        };

        std::array<std::atomic<bool>, zlstate::kBandNUM> is_bypassed_{};
        std::array<std::atomic<zlp::lrType::lrTypes>, zlstate::kBandNUM> lr_types_;

        std::atomic<bool> to_repaint_{false};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void updateCurveThickness();
    };
} // zlpanel
