// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_value_panel.hpp"
#include "../../dsp/dsp.hpp"

namespace zlPanel {
    OutputValuePanel::OutputValuePanel(PluginProcessor &p,
                                       zlInterface::UIBase &base)
        : processorRef(p),
          parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          scale(*parametersRef.getRawParameterValue(zlDSP::scale::ID)) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        lmPara = parametersRef.getParameter(zlDSP::loudnessMatcherON::ID);
        lookAndFeelChanged();
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);

        uiBase.getBoxTree().addListener(this);
    }

    OutputValuePanel::~OutputValuePanel() {
        uiBase.getBoxTree().removeListener(this);
        stopTimer(0);
    }

    void OutputValuePanel::paint(juce::Graphics &g) {
        g.setFont(uiBase.getFontSize() * 1.375f);
        if (showGain) {
            g.setColour(uiBase.getColourByIdx(zlInterface::gainColour));
            g.drawText(gainString, gainBound, juce::Justification::centred);
            g.drawText(scaleString, scaleBound, juce::Justification::centred);
        } else {
            if (static_cast<bool>(uiBase.getBoxProperty(zlInterface::boxIdx::outputBox))) {
                g.setColour(uiBase.getTextColor());
            } else {
                g.setColour(uiBase.getTextColor().withMultipliedAlpha(.75f));
            }
            g.drawText("Output", getLocalBounds().toFloat(), juce::Justification::centred);
        }
    }

    void OutputValuePanel::timerCallback(const int timerID) {
        if (timerID == 0) {
            updateGainValue();
        }
    }

    void OutputValuePanel::lookAndFeelChanged() {
        if (uiBase.getColourByIdx(zlInterface::gainColour).getAlpha() > juce::uint8(0)) {
            showGain = true;
            startTimer(0, 1500);
        } else {
            stopTimer(0);
            showGain = false;
            repaint();
        }
    }

    void OutputValuePanel::resized() {
        gainBound = getLocalBounds().toFloat();
        scaleBound = gainBound.removeFromRight(gainBound.getWidth() * .5f);
        const auto bound = getLocalBounds().toFloat();
        backgroundPath.clear();
        backgroundPath.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                           uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                           false, false, true, true);
    }

    void OutputValuePanel::updateGainValue() {
        const auto newGain = static_cast<float>(processorRef.getController().getGainCompensation());
        const auto newScale = scale.load();
        const auto newLearning = lmPara->getValue() > .5f;
        if (std::abs(newGain - currentGain) > 0.0 || std::abs(newScale - currentScale) > 0.0 ||
            newLearning != currentLearning) {
            currentGain = newGain;
            currentScale = newScale;
            currentLearning = newLearning;
            if (!currentLearning) {
                if (currentGain <= 0.04) {
                    gainString = juce::String(currentGain, 1, false);
                } else {
                    gainString = "+" + juce::String(currentGain, 1, false);
                }
            } else {
                gainString = "L";
            }
            scaleString = juce::String(static_cast<int>(std::round(scale.load()))) + "%";
            repaint();
        }
    }

    void OutputValuePanel::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                    const juce::Identifier &property) {
        juce::ignoreUnused(treeWhosePropertyHasChanged);
        if (zlInterface::UIBase::isBoxProperty(zlInterface::boxIdx::outputBox, property)) {
            repaint();
        }
    }
} // zlPanel
