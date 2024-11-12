// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SOLO_PANEL_HPP
#define ZLEqualizer_SOLO_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class SoloPanel final : public juce::Component {
    public:
        SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                  juce::AudioProcessorValueTreeState &parametersNA,
                  zlInterface::UIBase &base,
                  zlDSP::Controller<double> &controller);

        ~SoloPanel() override;

        void paint(juce::Graphics &g) override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;
        zlFilter::IIR<double, zlDSP::Controller<double>::FilterSize> &soloF;
        zlDSP::Controller<double> &controllerRef;
        std::atomic<float> scale1{.5f}, scale2{.5f};

        void handleAsyncUpdate();
    };
} // zlPanel

#endif //ZLEqualizer_SOLO_PANEL_HPP
