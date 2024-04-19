// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_GRID_PANEL_HPP
#define ZLEqualizer_GRID_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../gui/gui.hpp"

namespace zlPanel {
    class GridPanel final : public juce::Component {
    public:
        /** stl does not support constexpr log/pow,
         * (np.log([20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000]) - np.log(10)) / (np.log(22000) - np.log(10)) */
        static constexpr std::array<float, 10> backgroundFreqs = {
            0.09006341f, 0.20912077f, 0.29918418f, 0.3892476f, 0.50830495f,
            0.59836837f, 0.68843178f, 0.80748914f, 0.89755255f, 0.98761596f
        };

        inline static const std::array<juce::String, 10> backgroundFreqsNames = {
            "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"
        };

        static constexpr std::array<float, 6> backgroundDBs = {
            0.f, 1.f / 6.f, 2.f / 6.f, 0.5, 4.f / 6.f, 5.f / 6.f
        };

        explicit GridPanel(zlInterface::UIBase &base);

        ~GridPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        juce::RectangleList<float> rectList;
    };
}

#endif //ZLEqualizer_GRID_PANEL_HPP
