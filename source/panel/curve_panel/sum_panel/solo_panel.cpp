// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

#include "../../../state/state_definitions.hpp"

namespace zlpanel {
    SoloPanel::SoloPanel(juce::AudioProcessorValueTreeState &parameters,
                         juce::AudioProcessorValueTreeState &parameters_NA,
                         zlgui::UIBase &base,
                         zlp::Controller<double> &controller,
                         ButtonPanel &buttonPanel)
        : parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          soloF(controller.getSoloFilter()),
          controller_ref_(controller), buttonPanelRef(buttonPanel) {
        juce::ignoreUnused(parameters_ref_, parameters_NA);
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            soloUpdaters.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::appendSuffix(zlp::solo::ID, i)));
            sideSoloUpdaters.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::appendSuffix(zlp::sideSolo::ID, i)));
        }
        selectBandIdx.store(static_cast<size_t>(
            parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load()));
        handleAsyncUpdate();
    }

    SoloPanel::~SoloPanel() {
        turnOffSolo();
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
    }

    void SoloPanel::paint(juce::Graphics &g) {
        const size_t bandIdx = selectBandIdx.load();

        g.setColour(ui_base_.getTextColor().withAlpha(.1f));
        auto bound = getLocalBounds().toFloat();
        if (controller_ref_.getSoloIsSide()) {
            const auto x =  buttonPanelRef.getSideDragger(
                selectBandIdx.load()).getButton().getBoundsInParent().toFloat().getCentreX();
            if (std::abs(x - currentX) >= 0.001 || std::abs(soloF.getQ() - soloQ) >= 0.001) {
                currentX = x;
                handleAsyncUpdate();
            }
            const auto boundWidth = bound.getWidth();
            const auto leftWidth = currentX - currentBW * boundWidth;
            const auto rightWidth = boundWidth - currentX - currentBW * boundWidth;
            const auto leftArea = bound.removeFromLeft(leftWidth);
            const auto rightArea = bound.removeFromRight(rightWidth);
            g.fillRect(leftArea);
            g.fillRect(rightArea);
        } else {
            const auto x = buttonPanelRef.getDragger(selectBandIdx.load()
                ).getButton().getBoundsInParent().toFloat().getCentreX();
            if (std::abs(x - currentX) >= 0.001 || std::abs(soloF.getQ() - soloQ) >= 0.001) {
                currentX = x;
                handleAsyncUpdate();
            }
            const auto &f = controller_ref_.getMainIdealFilter(bandIdx);
            switch (f.getFilterType()) {
                case zldsp::filter::kHighPass:
                case zldsp::filter::kLowShelf: {
                    bound.removeFromLeft(currentX);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::kLowPass:
                case zldsp::filter::kHighShelf: {
                    bound = bound.removeFromLeft(currentX);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::kTiltShelf: {
                    break;
                }
                case zldsp::filter::kPeak:
                case zldsp::filter::kBandShelf:
                case zldsp::filter::kBandPass:
                case zldsp::filter::kNotch: {
                    const auto boundWidth = bound.getWidth();
                    const auto leftWidth = currentX - currentBW * boundWidth;
                    const auto rightWidth = boundWidth - currentX - currentBW * boundWidth;
                    const auto leftArea = bound.removeFromLeft(leftWidth);
                    const auto rightArea = bound.removeFromRight(rightWidth);
                    g.fillRect(leftArea);
                    g.fillRect(rightArea);
                }
            }
        }
    }

    void SoloPanel::handleAsyncUpdate() {
        soloQ = soloF.getQ();
        const auto bw = std::asinh(0.5f / soloQ);
        currentBW = static_cast<float>(bw) / std::log(2200.f);
    }

    void SoloPanel::parameterChanged(const juce::String &parameterID, const float newValue) {
        juce::ignoreUnused(parameterID);
        const auto previousBandIdx = selectBandIdx.load();
        const auto currentBandIdx = static_cast<size_t>(newValue);
        if (previousBandIdx != currentBandIdx) {
            soloUpdaters[previousBandIdx]->update(0.f);
            sideSoloUpdaters[previousBandIdx]->update(0.f);
        }
        selectBandIdx.store(currentBandIdx);
    }

    void SoloPanel::turnOffSolo() const {
        for (size_t i = 0; i < zlp::kBandNUM; ++i) {
            soloUpdaters[i]->updateSync(0.f);
            sideSoloUpdaters[i]->updateSync(0.f);
        }
    }
} // zlpanel
