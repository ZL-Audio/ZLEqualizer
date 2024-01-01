// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONTROL_PANEL_HPP
#define ZLEqualizer_CONTROL_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "../../gui/gui.h"

namespace zlPanel {
    class ControlPanel : public juce::Component {
    public:
        explicit ControlPanel(juce::AudioProcessorValueTreeState &parameters,
                              juce::AudioProcessorValueTreeState &parametersNA,
                              zlInterface::UIBase &base);

        ~ControlPanel() override;

        void resized() override;

    private:
        zlInterface::CompactButton bypassC, soloC, dynONC, dynBypassC, dynSoloC;
        zlInterface::CompactCombobox fTypeC, slopeC, stereoC;
        zlInterface::TwoValueRotarySlider freqC, gainC, qC, sideFreqC, sideQC;
        zlInterface::CompactLinearSlider thresC, ratioC, attackC, releaseC;
    };
} // zlPanel

#endif //ZLEqualizer_CONTROL_PANEL_HPP
