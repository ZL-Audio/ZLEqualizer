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
#include "../button_panel/button_panel.hpp"

namespace zlpanel {
    class SoloPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener {
    public:
        SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                  juce::AudioProcessorValueTreeState &parameters_NA,
                  zlgui::UIBase &base,
                  zlp::Controller<double> &controller,
                  ButtonPanel &buttonPanel);

        ~SoloPanel() override;

        void paint(juce::Graphics &g) override;

        void checkVisible() {
            setVisible(controller_ref_.getSolo());
        }

        void turnOffSolo() const;

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &uiBase;
        zldsp::filter::IIR<double, zlp::Controller<double>::kFilterSize> &soloF;
        zlp::Controller<double> &controller_ref_;
        ButtonPanel &buttonPanelRef;
        float currentX{0.}, currentBW{0.};
        double soloQ{0.};
        std::atomic<size_t> selectBandIdx{0};
        std::vector<std::unique_ptr<zldsp::chore::ParaUpdater> > soloUpdaters, sideSoloUpdaters;

        void handleAsyncUpdate();

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlpanel
