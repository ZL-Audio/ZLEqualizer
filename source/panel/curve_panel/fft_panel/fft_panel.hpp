// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_FFT_PANEL_HPP
#define ZLEqualizer_FFT_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"

namespace zlPanel {
    class FFTPanel final : public juce::Component {
    public:
        explicit FFTPanel(zlFFT::PrePostFFTAnalyzer<double> &analyzer,
                          zlInterface::UIBase &base);

        ~FFTPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updatePaths();

    private:
        zlFFT::PrePostFFTAnalyzer<double> &analyzerRef;
        zlInterface::UIBase &uiBase;
        juce::Path path1{}, path2{}, path3{};
        juce::Path recentPath1{}, recentPath2{}, recentPath3{};
        juce::SpinLock pathLock;
        juce::Point<float> leftCorner, rightCorner;
        AtomicBound atomicBound;
        std::atomic<bool> firstPath = true;
    };
} // zlPanel

#endif //ZLEqualizer_FFT_PANEL_HPP
