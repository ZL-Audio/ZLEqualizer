// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"

namespace zlpanel {
    class FFTPanel final : public juce::Component {
    public:
        explicit FFTPanel(zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzer,
                          zlgui::UIBase &base);

        ~FFTPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updatePaths(float physicalPixelScaleFactor);

        void visibilityChanged() override;

        void setMinimumFFTDB(const float x) {
            minimumFFTDB.store(x);
        }

    private:
        zldsp::analyzer::PrePostFFTAnalyzer<double> &analyzerRef;
        zlgui::UIBase &ui_base_;
        juce::Path prePath{}, postPath{}, postStrokePath{}, sidePath{};
        juce::Path recentPrePath{}, recentPostPath{}, recentPostStrokePath{}, recentSidePath{};
        juce::SpinLock pathLock;
        AtomicPoint<float> leftCorner, rightCorner;
        AtomicBound<float> atomicBound;
        std::atomic<float> curveThickness{0.f};
        std::atomic<bool> firstPath = true;
        std::atomic<float> minimumFFTDB{-72.f};
    };
} // zlpanel
