// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_BACKGROUND_PANEL_HPP
#define ZLEqualizer_BACKGROUND_PANEL_HPP

#include "../../../gui/gui.hpp"
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlPanel {
    class BackgroundPanel final : public juce::Component {
    public:
        /** stl does not support constexpr log/pow,
         * (np.log([20, 50, 100, 200, 500, 1000, 2000, 5000, 10000]) - np.log(10)) / (np.log(20000) - np.log(10)) */
        static constexpr std::array<float, 9> backgroundFreqs = {
            0.09119275f, 0.211743f, 0.30293575f, 0.3941285f, 0.51467875f,
            0.6058715f, 0.69706425f, 0.8176145f, 0.90880725f
        };

        inline static const std::array<juce::String, 9> backgroundFreqsNames = {
            "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k"
        };

        static constexpr std::array<float, 4> backgroundDBs = {
            0.f, 0.25f, 0.5, 0.75f
        };

        explicit BackgroundPanel(zlInterface::UIBase &base);

        ~BackgroundPanel() override;

        void paint(juce::Graphics &g) override;

        // void resized() override;

    private:
        zlInterface::UIBase &uiBase;
    };
}

#endif //ZLEqualizer_BACKGROUND_PANEL_HPP
