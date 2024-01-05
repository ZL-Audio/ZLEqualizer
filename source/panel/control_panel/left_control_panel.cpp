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
                                       zlInterface::UIBase &base)
        : uiBase(base),
          parametersRef(parameters),
          parametersNARef(parametersNA),
          bypassC("B", base),
          soloC("S", base), dynONC("D", base),
          fTypeC("", zlDSP::fType::choices, base),
          slopeC("", zlDSP::slope::choices, base),
          stereoC("", zlDSP::lrType::choices, base),
          freqC("FREQ", base),
          gainC("GAIN", base),
          qC("Q", base) {
        juce::ignoreUnused(parametersNA, parametersNARef);
        attachGroup(0);
    }

    LeftControlPanel::~LeftControlPanel() {
        for (size_t i = 0; i < zlDSP::bandNUM; ++i) {
            const std::string suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            parametersRef.removeParameterListener(zlDSP::fType::ID + suffix, this);
            parametersRef.removeParameterListener(zlDSP::dynamicON::ID + suffix, this);
        }
    }

    void LeftControlPanel::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void LeftControlPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(4)), Track(Fr(6)),
            Track(Fr(6)), Track(Fr(6)), Track(Fr(6)), Track(Fr(5))
        };
        grid.items = {
            juce::GridItem(bypassC).withArea(1, 1),
            juce::GridItem(fTypeC).withArea(1, 2),
            juce::GridItem(freqC).withArea(1, 3, 3, 4),
            juce::GridItem(gainC).withArea(1, 4, 3, 5),
            juce::GridItem(qC).withArea(1, 5, 3, 6),
            juce::GridItem(stereoC).withArea(1, 6),
            juce::GridItem(soloC).withArea(2, 1),
            juce::GridItem(slopeC).withArea(2, 2),
            juce::GridItem(dynONC).withArea(2, 6),
        };
        for (auto &s: {&freqC, &gainC, &qC}) {
            s->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }

        auto bound = getLocalBounds().toFloat();
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
    }

    void LeftControlPanel::attachGroup(const size_t idx) {
        const std::string oldSuffix = bandIdx.load() < 10
                                          ? "0" + std::to_string(bandIdx.load())
                                          : std::to_string(bandIdx.load());
        parametersRef.removeParameterListener(zlDSP::fType::ID + oldSuffix, this);
        parametersRef.removeParameterListener(zlDSP::dynamicON::ID + oldSuffix, this);

        bandIdx.store(idx);
        const std::string suffix = idx < 10 ? "0" + std::to_string(idx) : std::to_string(idx);
        parametersRef.addParameterListener(zlDSP::fType::ID + suffix, this);
        parametersRef.addParameterListener(zlDSP::dynamicON::ID + suffix, this);

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
        const auto id = parameterID.dropLastCharacters(2);
        const auto idx = static_cast<size_t>(parameterID.getTrailingIntValue());
        if (id == zlDSP::fType::ID) {
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
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        } else if (id == zlDSP::dynamicON::ID) {
            const auto f = static_cast<bool>(newValue);
            gainC.setShowSlider2(f);
            qC.setShowSlider2(f);
            if (idx == bandIdx.load()) {
                triggerAsyncUpdate();
            }
        }
    }

    void LeftControlPanel::handleAsyncUpdate() {
        repaint();
    }
}
