// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlPanel {
    LeftControlPanel::LeftControlPanel(juce::AudioProcessorValueTreeState &parameters,
                                       juce::AudioProcessorValueTreeState &parametersNA,
                                       zlInterface::UIBase &base) : uiBase(base),
                                                                    parametersRef(parameters),
                                                                    parametersNARef(parametersNA),
                                                                    bypassC("B", base),
                                                                    soloC("S", base), dynONC("D", base),
                                                                    fTypeC("", {
                                                                               "Bell", "Low Shelf", "Low Pass",
                                                                               "High Self", "High Pass", "Notch",
                                                                               "Band Pass", "Band Shelf", "Tilt"
                                                                           }, base),
                                                                    slopeC("", {
                                                                               "6 dB/oct", "12 dB/oct", "24 dB/oct",
                                                                               "36 dB/oct", "48 dB/oct", "72 dB/oct",
                                                                               "96 dB/oct"
                                                                           }, base),
                                                                    stereoC("", {"S", "L", "R", "M", "S"}, base),
                                                                    freqC("FREQ", base),
                                                                    gainC("GAIN", base),
                                                                    qC("Q", base) {
        juce::ignoreUnused(parametersNA, parametersNARef);
        attachGroup(0);
    }

    LeftControlPanel::~LeftControlPanel() = default;

    void LeftControlPanel::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void LeftControlPanel::resized() {
    }

    void LeftControlPanel::attachGroup(const size_t idx) {
        const std::string oldSuffix = bandIdx.load() < 10
                                          ? "0" + std::to_string(bandIdx.load())
                                          : std::to_string(bandIdx.load());
        parametersRef.removeParameterListener(zlDSP::fType::ID + oldSuffix, this);

        bandIdx.store(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parametersRef.addParameterListener(zlDSP::fType::ID + suffix, this);

        buttonAttachments.clear(true);
        boxAttachments.clear(true);
        sliderAttachments.clear(true);

        attach(*this, {&bypassC, &soloC, &dynONC},
               {zlDSP::bypass::ID + suffix, zlDSP::solo::ID + suffix, zlDSP::dynamicON::ID + suffix},
               parametersRef, buttonAttachments);
        attach(*this, {&fTypeC, &slopeC, &stereoC},
               {zlDSP::fType::ID + suffix, zlDSP::slope::ID + suffix, zlDSP::lrType::ID + suffix},
               parametersRef, boxAttachments);
        attach(*this, {&freqC, &gainC, &qC},
               {
                   zlDSP::freq::ID + suffix, "", zlDSP::gain::ID + suffix, zlDSP::targetGain::ID + suffix,
                   zlDSP::Q::ID + suffix, zlDSP::targetQ::ID + suffix
               },
               parametersRef, sliderAttachments);
    }

    void LeftControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID);
        switch (const auto fType = static_cast<zlIIR::FilterType>(newValue)) {
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::lowShelf:
            case zlIIR::FilterType::highShelf:
            case zlIIR::FilterType::bandShelf:
            case zlIIR::FilterType::tiltShelf:
                gainC.setEditable(true);
                break;
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::bandPass:
            case zlIIR::FilterType::notch:
                gainC.setEditable(false);
                break;
        }
        triggerAsyncUpdate();
    }

    void LeftControlPanel::handleAsyncUpdate() {
        repaint();
    }
}
