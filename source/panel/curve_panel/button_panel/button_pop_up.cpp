// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_pop_up.hpp"

namespace zlPanel {
    ButtonPopUp::ButtonPopUp(const size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : band{bandIdx}, parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          background(bandIdx, parameters, parametersNA, base), pitchLabel(base) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        freqPara = parametersRef.getParameter(zlDSP::appendSuffix(zlDSP::freq::ID, band));

        addAndMakeVisible(background);

        addAndMakeVisible(pitchLabel);
    }

    ButtonPopUp::~ButtonPopUp() {
        pitchLabel.setLookAndFeel(nullptr);
    }

    void ButtonPopUp::resized() {
        background.setBounds(getLocalBounds());

        auto bound = getLocalBounds().toFloat();
        bound.removeFromBottom(bound.getHeight() * .4f);
        bound.removeFromLeft(bound.getWidth() * .705882f);
        bound.removeFromRight(uiBase.getFontSize() * .25f);

        pitchLabel.setBounds(bound.toNearestInt());
    }

    void ButtonPopUp::updateBounds(const juce::Component &component) {
        if (getParentComponent() == nullptr || component.getParentComponent() == nullptr) {
            return;
        }
        const auto compBound = component.getBoundsInParent().toFloat();
        const auto compParentBound = component.getParentComponent()->getLocalBounds().toFloat();
        const auto shiftX = compBound.getCentreX() - compParentBound.getCentreX();
        const auto shiftY = compBound.getCentreY() - compParentBound.getCentreY();
        const auto shiftYPortion = shiftY / (compParentBound.getHeight() - uiBase.getFontSize()) * 2;

        switch (fType.load()) {
            case zlFilter::FilterType::peak:
            case zlFilter::FilterType::bandShelf: {
                if (direction > 0.f) {
                    if (shiftYPortion > .5f || (shiftYPortion < -0.1f && shiftYPortion > -0.4f)) {
                        direction = -1.f;
                    }
                } else {
                    if (shiftYPortion < -0.5f || (shiftYPortion > 0.1f && shiftYPortion < 0.4f)) {
                        direction = 1.f;
                    }
                }
                break;
            }
            case zlFilter::FilterType::lowShelf:
            case zlFilter::FilterType::highShelf:
            case zlFilter::FilterType::tiltShelf: {
                if (direction > 0.f && shiftYPortion < -0.2f) {
                        direction = -1.f;
                } else if (direction < 0.f && shiftYPortion > 0.2f) {
                        direction = 1.f;
                }
                break;
            }
            case zlFilter::FilterType::notch:
            case zlFilter::FilterType::lowPass:
            case zlFilter::FilterType::highPass:
            case zlFilter::FilterType::bandPass: {
                direction = -1.f;
                break;
            }
        }

        const auto bound = getParentComponent()->getLocalBounds().toFloat();
        const auto finalY = bound.getCentreY() + direction * height * uiBase.getFontSize() + shiftY;
        const auto finalX = juce::jlimit(bound.getX() + width * uiBase.getFontSize() / 2,
                                         bound.getRight() - width * uiBase.getFontSize() / 2,
                                         bound.getCentreX() + shiftX);

       const auto popUpBound = juce::Rectangle<float>(
            width * uiBase.getFontSize(),
            height * uiBase.getFontSize()).withCentre({finalX, finalY});

        updateLabel();
        setBounds(popUpBound.toNearestInt());
    }

    void ButtonPopUp::updateLabel() {
        const auto freq = zlDSP::freq::range.convertFrom0to1(freqPara->getValue());
        const auto pitchIdx = juce::roundToInt(12 * std::log2(freq / 440.f));
        const auto pitchIdx1 = (pitchIdx + 240) % 12;
        const auto pitchIdx2 = (pitchIdx + 240) / 12 - 16;
        const auto pitchString = pitchIdx2 >= 0
                                     ? std::string(pitchLookUp[static_cast<size_t>(pitchIdx1)]) + std::to_string(pitchIdx2)
                                     : "A0";
        pitchLabel.setText(pitchString);
    }
} // zlPanel
