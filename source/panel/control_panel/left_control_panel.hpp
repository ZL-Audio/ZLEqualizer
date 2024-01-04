// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_LEFT_CONTROL_PANEL_HPP
#define ZLEqualizer_LEFT_CONTROL_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "../../gui/gui.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    class LeftControlPanel final : public juce::Component,
                                   private juce::AudioProcessorValueTreeState::Listener,
                                   private juce::AsyncUpdater {
    public:
        explicit LeftControlPanel(juce::AudioProcessorValueTreeState &parameters,
                                  juce::AudioProcessorValueTreeState &parametersNA,
                                  zlInterface::UIBase &base);

        ~LeftControlPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;

        zlInterface::CompactButton bypassC, soloC, dynONC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;

        zlInterface::CompactCombobox fTypeC, slopeC, stereoC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        zlInterface::TwoValueRotarySlider freqC, gainC, qC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;

        std::atomic<size_t> bandIdx;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void attachGroup(size_t idx);
    };
}


#endif //ZLEqualizer_LEFT_CONTROL_PANEL_HPP
