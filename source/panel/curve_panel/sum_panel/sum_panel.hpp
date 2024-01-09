// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SUM_PANEL_HPP
#define ZLEqualizer_SUM_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class SumPanel final : public juce::Component, private juce::Timer {
    public:
        explicit SumPanel(zlInterface::UIBase &base, zlDSP::Controller<float> &controller);

        ~SumPanel() override;

        void paint(juce::Graphics &g) override;

        void setMaximumDB(const float x) { maximumDB.store(x);}

    private:
        juce::Path path;
        zlInterface::UIBase &uiBase;
        zlDSP::Controller<float> &c;
        std::atomic<float> maximumDB;

        void timerCallback() override;
    };
} // zlPanel

#endif //ZLEqualizer_SUM_PANEL_HPP
