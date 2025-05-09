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
                          std::array<zldsp::filter::Ideal<double, 16>, 16> &baseFilters,
                          std::array<zldsp::filter::Ideal<double, 16>, 16> &mainFilters);

        ~SumPanel() override;

        void paint(juce::Graphics &g) override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            toRepaint.store(true);
        }

        bool checkRepaint();

        void resized() override;

        void run(float physicalPixelScaleFactor);

        void lookAndFeelChanged() override;

    private:
        std::array<juce::Path, 5> paths, recentPaths, strokePaths;
        std::array<juce::SpinLock, 5> pathLocks;
        std::array<juce::Colour, 5> colours;
        juce::AudioProcessorValueTreeState &parametersRef;
        zlgui::UIBase &uiBase;
        zlp::Controller<double> &c;
        std::array<zldsp::filter::Ideal<double, 16>, zlstate::bandNUM> &mMainFilters;
        std::atomic<float> maximumDB;
        std::vector<double> dBs{};
        AtomicBound<float> atomicBound;
        std::atomic<float> curveThickness{0.f};

        static constexpr std::array changeIDs{
            zlp::bypass::ID, zlp::lrType::ID
        };

        std::array<std::atomic<bool>, zlstate::bandNUM> isBypassed{};
        std::array<std::atomic<zlp::lrType::lrTypes>, zlstate::bandNUM> lrTypes;

        std::atomic<bool> toRepaint{false};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updateCurveThickness();
    };
} // zlpanel
