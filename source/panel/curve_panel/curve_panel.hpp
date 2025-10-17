// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "background_panel/background_panel.hpp"
#include "fft_panel/fft_panel.hpp"
#include "response_panel/response_panel.hpp"
#include "scale_panel/scale_panel.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component {
    public:
        explicit CurvePanel(PluginProcessor& p, zlgui::UIBase& base,
                            const multilingual::TooltipHelper& tooltip_helper);

        void paintOverChildren(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBack();

        void repaintCallBackSlow();

        void updateBand();

        void updateSampleRate(double sample_rate);

        void startThreads();

        void stopThreads();

    private:
        BackgroundPanel background_panel_;
        FFTPanel fft_panel_;
        ResponsePanel response_panel_;
        ScalePanel scale_panel_;
    };
}
