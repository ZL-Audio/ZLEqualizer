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

namespace zlpanel {
    OutputValuePanel::OutputValuePanel(PluginProcessor &p,
                                       zlgui::UIBase &base)
        : processor_ref_(p),
          parameters_ref_(p.parameters),
          parameters_NA_ref_(p.parameters_NA),
          ui_base_(base),
          scale(*parameters_ref_.getRawParameterValue(zlp::scale::ID)) {
        juce::ignoreUnused(parameters_ref_, parameters_NA_ref_);
        lmPara = parameters_ref_.getParameter(zlp::loudnessMatcherON::ID);
        lookAndFeelChanged();
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);

        ui_base_.getBoxTree().addListener(this);
    }

    OutputValuePanel::~OutputValuePanel() {
        ui_base_.getBoxTree().removeListener(this);
        stopTimer(0);
    }

    void OutputValuePanel::paint(juce::Graphics &g) {
        g.setFont(ui_base_.getFontSize() * 1.375f);
        if (showGain) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kGainColour));
            g.drawText(gainString, gainBound, juce::Justification::centred);
            g.drawText(scaleString, scaleBound, juce::Justification::centred);
        } else {
            if (static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kOutputBox))) {
                g.setColour(ui_base_.getTextColor());
            } else {
                g.setColour(ui_base_.getTextColor().withMultipliedAlpha(.75f));
            }
            g.drawText("Output", getLocalBounds().toFloat(), juce::Justification::centred);
        }
    }

    void OutputValuePanel::timerCallback(const int timerID) {
        if (!ui_base_.getIsEditorShowing()) return;
        if (timerID == 0) {
            updateGainValue();
        }
    }

    void OutputValuePanel::lookAndFeelChanged() {
        if (ui_base_.getColourByIdx(zlgui::kGainColour).getAlpha() > juce::uint8(0)) {
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
                                           ui_base_.getFontSize() * .5f, ui_base_.getFontSize() * .5f,
                                           false, false, true, true);
    }

    void OutputValuePanel::updateGainValue() {
        const auto newGain = static_cast<float>(processor_ref_.getController().getGainCompensation());
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
        if (zlgui::UIBase::isBoxProperty(zlgui::BoxIdx::kOutputBox, property)) {
            repaint();
        }
    }
} // zlpanel
