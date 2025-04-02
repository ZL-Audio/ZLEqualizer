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
    ButtonPopUp::PitchLabel::PitchLabel(zlInterface::UIBase &base, juce::RangedAudioParameter *freq)
                : uiBase(base), freqPara(freq), laf(uiBase) {
        label.setLookAndFeel(&laf);
        label.setJustificationType(juce::Justification::centredRight);
        laf.setFontScale(1.2f);
        label.setEditable(false, true);
        label.addListener(this);
        addAndMakeVisible(label);
        label.setBufferedToImage(true);
    }

    void ButtonPopUp::PitchLabel::setFreq(const double freq) {
        const auto pitchIdx = juce::roundToInt(12 * std::log2(freq / 440.f));
        const auto pitchIdx1 = (pitchIdx + 240) % 12;
        const auto pitchIdx2 = (pitchIdx + 240) / 12 - 16;
        const auto pitchString = pitchIdx2 >= 0
                                     ? std::string(pitchLookUp[static_cast<size_t>(pitchIdx1)]) + std::to_string(
                                           pitchIdx2)
                                     : "A0";
        label.setText(pitchString, juce::dontSendNotification);
    }

    void ButtonPopUp::PitchLabel::editorShown(juce::Label *, juce::TextEditor &editor) {
        editor.setInputRestrictions(0, "0123456789.kKABCDEFGabcdefg#");

        editor.setJustification(juce::Justification::centredRight);
        editor.setColour(juce::TextEditor::outlineColourId, uiBase.getTextColor());
        editor.setColour(juce::TextEditor::highlightedTextColourId, uiBase.getTextColor());
        editor.applyFontToAllText(juce::FontOptions{uiBase.getFontSize() * 1.2f});
        editor.applyColourToAllText(uiBase.getTextColor(), true);

        editor.addListener(this);
        hasEditorChanged = false;
    }

    void ButtonPopUp::PitchLabel::editorHidden(juce::Label *, juce::TextEditor &editor) {
        editor.removeListener(this);
        if (!hasEditorChanged) return;
        const auto s = editor.getText();
        double value;
        if (const auto v = parseFreqPitchString(s)) {
            value = v.value();
        } else {
            value = parseFreqValueString(s);
        }
        freqPara->beginChangeGesture();
        freqPara->setValueNotifyingHost(freqPara->convertTo0to1(static_cast<float>(value)));
        freqPara->endChangeGesture();

        setFreq(value);
    }

    void ButtonPopUp::PitchLabel::textEditorTextChanged(juce::TextEditor &) {
        hasEditorChanged = true;
    }

    ButtonPopUp::ButtonPopUp(const size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA, zlInterface::UIBase &base)
        : band{bandIdx}, parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          fType(*parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::fType::ID, band))),
          freqPara(*parametersRef.getRawParameterValue(zlDSP::appendSuffix(zlDSP::freq::ID, band))),
          background(bandIdx, parameters, parametersNA, base),
          pitchLabel(base, parameters.getParameter(zlDSP::appendSuffix(zlDSP::freq::ID, bandIdx))) {
        juce::ignoreUnused(parametersRef, parametersNARef);

        addAndMakeVisible(background);
        addAndMakeVisible(pitchLabel);
    }

    ButtonPopUp::~ButtonPopUp() {
        pitchLabel.setLookAndFeel(nullptr);
    }

    void ButtonPopUp::resized() {
        const auto currentBound = getLocalBounds();
        background.setBounds(currentBound);

        auto bound = currentBound.toFloat();
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

        switch (static_cast<zlFilter::FilterType>(fType.load())) {
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

        const auto width = widthP * uiBase.getFontSize();
        const auto height = heightP * uiBase.getFontSize();
        const auto bound = getParentComponent()->getLocalBounds().toFloat();
        const auto finalY = bound.getCentreY() + direction * height + shiftY;
        const auto finalX = juce::jlimit(bound.getX() + width * .5f,
                                         bound.getRight() - width * .5f,
                                         bound.getCentreX() + shiftX);

        const auto popUpBound = juce::Rectangle<float>(width, height).withCentre({finalX, finalY});

        const auto actualBound = juce::Rectangle<int>(
            juce::roundToInt(popUpBound.getX()), juce::roundToInt(popUpBound.getY()),
            juce::roundToInt(width), juce::roundToInt(height));

        if (actualBound != previousBound) {
            previousBound = actualBound;
            updateLabel();
            setBounds(actualBound);
        }
    }

    void ButtonPopUp::updateLabel() {
        pitchLabel.setFreq(freqPara.load());
    }
} // zlPanel
