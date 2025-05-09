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
                         juce::AudioProcessorValueTreeState &parametersNA,
                         zlgui::UIBase &base,
                         zlp::Controller<double> &controller,
                         ButtonPanel &buttonPanel)
        : parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          soloF(controller.getSoloFilter()),
          controllerRef(controller), buttonPanelRef(buttonPanel) {
        juce::ignoreUnused(parametersRef, parametersNA);
        parametersNARef.addParameterListener(zlstate::selectedBandIdx::ID, this);
        for (size_t i = 0; i < zlp::bandNUM; ++i) {
            soloUpdaters.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parametersRef, zlp::appendSuffix(zlp::solo::ID, i)));
            sideSoloUpdaters.emplace_back(std::make_unique<zldsp::chore::ParaUpdater>(
                parametersRef, zlp::appendSuffix(zlp::sideSolo::ID, i)));
        }
        selectBandIdx.store(static_cast<size_t>(
            parametersNARef.getRawParameterValue(zlstate::selectedBandIdx::ID)->load()));
        handleAsyncUpdate();
    }

    SoloPanel::~SoloPanel() {
        turnOffSolo();
        parametersNARef.removeParameterListener(zlstate::selectedBandIdx::ID, this);
    }

    void SoloPanel::paint(juce::Graphics &g) {
        const size_t bandIdx = selectBandIdx.load();

        g.setColour(uiBase.getTextColor().withAlpha(.1f));
        auto bound = getLocalBounds().toFloat();
        if (controllerRef.getSoloIsSide()) {
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
            const auto &f = controllerRef.getMainIdealFilter(bandIdx);
            switch (f.getFilterType()) {
                case zldsp::filter::highPass:
                case zldsp::filter::lowShelf: {
                    bound.removeFromLeft(currentX);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::lowPass:
                case zldsp::filter::highShelf: {
                    bound = bound.removeFromLeft(currentX);
                    g.fillRect(bound);
                    break;
                }
                case zldsp::filter::tiltShelf: {
                    break;
                }
                case zldsp::filter::peak:
                case zldsp::filter::bandShelf:
                case zldsp::filter::bandPass:
                case zldsp::filter::notch: {
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
        for (size_t i = 0; i < zlp::bandNUM; ++i) {
            soloUpdaters[i]->updateSync(0.f);
            sideSoloUpdaters[i]->updateSync(0.f);
        }
    }
} // zlpanel
