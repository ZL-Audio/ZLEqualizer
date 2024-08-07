// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_pop_up.hpp"

namespace zlPanel {
    ButtonPopUp::ButtonPopUp(const size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : band{bandIdx}, parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          bypassC("B", base),
          soloC("S", base),
          bypassDrawable(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg, BinaryData::fadpowerswitch_svgSize)),
          soloDrawable(juce::Drawable::createFromImageData(BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)),
          fTypeC("", zlDSP::fType::choices, base),
          pitchLAF(base),
          drawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          button(drawable.get(), base) {
        juce::ignoreUnused(parametersNARef);

        bypassC.getLAF().enableShadow(false);
        bypassC.getLAF().setReverse(true);
        soloC.getLAF().enableShadow(false);
        bypassC.setDrawable(bypassDrawable.get());
        soloC.setDrawable(soloDrawable.get());
        for (auto &c: {&bypassC, &soloC}) {
            addAndMakeVisible(c);
        }
        attach({&bypassC.getButton(), &soloC.getButton()},
               {
                   zlDSP::appendSuffix(zlDSP::bypass::ID, bandIdx),
                   zlDSP::appendSuffix(zlDSP::solo::ID, bandIdx)
               },
               parametersRef, buttonAttachments);

        fTypeC.getLAF().setFontScale(1.25f);
        for (auto &c: {&fTypeC}) {
            addAndMakeVisible(c);
        }
        attach({&fTypeC.getBox()},
               {zlDSP::appendSuffix(zlDSP::fType::ID, bandIdx)},
               parametersRef, boxAttachments);

        const auto freqId = zlDSP::appendSuffix(zlDSP::freq::ID, band.load());

        parametersRef.addParameterListener(freqId, this);
        parameterChanged(freqId, parametersRef.getRawParameterValue(freqId)->load());

        pitchLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(pitchLabel);

        button.getButton().onClick = [this]() {
            const auto activeID = zlState::appendSuffix(zlState::active::ID, band.load());
            parametersNARef.getParameter(activeID)->beginChangeGesture();
            parametersNARef.getParameter(activeID)->setValueNotifyingHost(static_cast<float>(false));
            parametersNARef.getParameter(activeID)->endChangeGesture();
        };
        button.getLookAndFeel().setCurve(false, true, false, false);
        addAndMakeVisible(button);
    }

    ButtonPopUp::~ButtonPopUp() {
        pitchLabel.setLookAndFeel(nullptr);
        parametersRef.removeParameterListener(zlDSP::appendSuffix(zlDSP::freq::ID, band.load()), this);
    }

    void ButtonPopUp::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setFont(uiBase.getFontSize() * 1.2f);
        g.setColour(uiBase.getTextColor());
        auto textBound = pitchLabel.getBoundsInParent().toFloat();
        textBound.removeFromRight(uiBase.getFontSize() * .25f);
        g.drawText(pitchString, textBound, juce::Justification::centredRight, false);
    }

    void ButtonPopUp::resized() {
        button.getLookAndFeel().setPadding(uiBase.getFontSize() * 0.65f);
        pitchLAF.setPadding(0, uiBase.getFontSize() * .25f, 0, 0);

        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(60)), Track(Fr(40))};
        grid.templateColumns = {
            Track(Fr(30)), Track(Fr(30)), Track(Fr(25))
        };
        grid.items = {
            juce::GridItem(bypassC).withArea(1, 1),
            juce::GridItem(soloC).withArea(1, 2),
            juce::GridItem(pitchLabel).withArea(1, 3),
            juce::GridItem(button).withArea(2, 3, 3, 4),
            juce::GridItem(fTypeC).withArea(2, 1, 3, 3)
        };

        const auto bound = getLocalBounds().toFloat();
        grid.performLayout(bound.toNearestInt());
    }

    void ButtonPopUp::componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) {
        juce::ignoreUnused(wasMoved, wasResized);
        if (getParentComponent() == nullptr || component.getParentComponent() == nullptr) {
            return;
        }
        const auto compBound = component.getBoundsInParent().toFloat();
        const auto compParentBound = component.getParentComponent()->getLocalBounds().toFloat();
        const auto shiftX = compBound.getCentreX() - compParentBound.getCentreX();
        const auto shiftY = compBound.getCentreY() - compParentBound.getCentreY();
        const auto shiftYPortion = shiftY / (compParentBound.getHeight() - uiBase.getFontSize()) * 2;
        auto direction = -1.f;
        switch (fType.load()) {
            case zlIIR::FilterType::peak:
            case zlIIR::FilterType::bandShelf: {
                direction = (shiftYPortion > 0.f && shiftYPortion < 0.5f) || (shiftYPortion < -0.5f) ? 1.f : -1.f;
                break;
            }
            case zlIIR::FilterType::lowShelf:
            case zlIIR::FilterType::highShelf:
            case zlIIR::FilterType::tiltShelf: {
                direction = (shiftYPortion > 0.f) ? 1.f : -1.f;
                break;
            }
            case zlIIR::FilterType::notch:
            case zlIIR::FilterType::lowPass:
            case zlIIR::FilterType::highPass:
            case zlIIR::FilterType::bandPass: {
                break;
            }
        }

        const auto bound = getParentComponent()->getLocalBounds().toFloat();
        const auto finalY = bound.getCentreY() + direction * height.load() * uiBase.getFontSize() + shiftY;
        const auto finalX = juce::jlimit(bound.getX() + width.load() * uiBase.getFontSize() / 2,
                                         bound.getRight() - width.load() * uiBase.getFontSize() / 2,
                                         bound.getCentreX() + shiftX);

        popUpBound = juce::Rectangle<float>(
            width.load() * uiBase.getFontSize(),
            height.load() * uiBase.getFontSize()).withCentre({finalX, finalY});
        toUpdateBounds.store(true);
    }

    void ButtonPopUp::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID);
        freq.store(newValue);
        triggerAsyncUpdate();
    }

    void ButtonPopUp::handleAsyncUpdate() {
        const auto pitchIdx = juce::roundToInt(12 * std::log2(freq.load() / 440.f));
        const auto pitchIdx1 = (pitchIdx + 240) % 12;
        const auto pitchIdx2 = (pitchIdx + 240) / 12 - 16;
        pitchString = pitchIdx2 >= 0
                                     ? std::string(pitchLookUp[static_cast<size_t>(pitchIdx1)]) + std::to_string(pitchIdx2)
                                     : "A0";
    }
} // zlPanel
