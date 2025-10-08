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
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class BackgroundPanel final : public juce::Component {
    public:
        explicit BackgroundPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        void updateSampleRate(double sample_rate);

    private:
        static constexpr std::array kFreqValues = {
            20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f,
            10000.f, 20000.f, 50000.f, 100000.f
        };

        zlgui::UIBase& base_;
        double freq_max_{0.};

        void drawFreqs(juce::Graphics& g);

        void drawDBs(juce::Graphics& g) const;
    };
}
