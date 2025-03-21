// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    ScalePanel::ScalePanel(PluginProcessor &processor, zlInterface::UIBase &base)
        : parametersNARef(processor.parametersNA), uiBase(base),
          scaleBox("", zlState::maximumDB::choices, base),
          minFFTBox("", zlState::minimumFFTDB::choices, base) {
        juce::ignoreUnused(uiBase);
        scaleBox.getLAF().setFontScale(zlInterface::FontLarge);
        scaleBox.getBox().setJustificationType(juce::Justification::centredRight);
        scaleBox.getBox().onChange = [this]() {
            handleAsyncUpdate();
        };
        minFFTBox.getLAF().setFontScale(zlInterface::FontLarge);
        minFFTBox.getBox().setJustificationType(juce::Justification::centredRight);
        minFFTBox.getBox().onChange = [this]() {
            handleAsyncUpdate();
        };
        attach({&scaleBox.getBox(), &minFFTBox.getBox()},
               {zlState::maximumDB::ID, zlState::minimumFFTDB::ID},
               parametersNARef, boxAttachments);
        addAndMakeVisible(scaleBox);
        addAndMakeVisible(minFFTBox);
        handleAsyncUpdate();

        SettableTooltipClient::setTooltip(uiBase.getToolTipText(zlInterface::multilingual::labels::dbScale));

        setBufferedToImage(true);
    }

    ScalePanel::~ScalePanel() = default;

    void ScalePanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());

        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());

        g.setFont(uiBase.getFontSize() * zlInterface::FontLarge);
        for (auto &d: scaleDBs) {
            const auto y = d * bound.getHeight() + bound.getY() - uiBase.getFontSize() * .75f;
            const auto lTextBound = juce::Rectangle<float>(bound.getX(), y, bound.getWidth() * .4f,
                                                           uiBase.getFontSize() * 1.5f);
            const auto leftDB = static_cast<int>(std::round(-2 * d * maximumDB.load() + maximumDB.load()));
            g.setColour(uiBase.getTextColor());
            g.drawText(juce::String(leftDB), lTextBound, juce::Justification::centredRight);
            const auto rTextBound = juce::Rectangle<float>(bound.getCentreX(), y, bound.getWidth() * .4f,
                                                           uiBase.getFontSize() * 1.5f);
            const auto fftDB = static_cast<int>(std::round(minimumFFTDB.load() * d));
            if (fftDB > -100) {
                g.setColour(uiBase.getTextInactiveColor());
                g.drawText(juce::String(fftDB), rTextBound, juce::Justification::centredRight);
            }
        }
    }

    void ScalePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * uiBase.getFontSize());
        {
            auto boxBound = juce::Rectangle<float>(uiBase.getFontSize() * 4.f, uiBase.getFontSize() * 1.5f);
            boxBound = boxBound.withCentre({bound.getCentreX(), bound.getY()});
            boxBound.removeFromRight(uiBase.getFontSize() * .95f);
            boxBound.removeFromLeft(uiBase.getFontSize() * .05f);
            scaleBox.setBounds(boxBound.toNearestInt());
        }
        {
            auto boxBound = juce::Rectangle<float>(bound.getWidth(), uiBase.getFontSize() * 1.5f);
            boxBound = boxBound.withCentre({bound.getCentreX(), bound.getBottom()});
            boxBound.removeFromRight(uiBase.getFontSize() * .5f);
            boxBound.removeFromLeft(uiBase.getFontSize() * .5f);
            minFFTBox.setBounds(boxBound.toNearestInt());
        }
    }

    void ScalePanel::handleAsyncUpdate() {
        maximumDB.store(zlState::maximumDB::dBs[
            static_cast<size_t>(parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load())]);
        minimumFFTDB.store(zlState::minimumFFTDB::dBs[
            static_cast<size_t>(parametersNARef.getRawParameterValue(zlState::minimumFFTDB::ID)->load())]);
        repaint();
    }
} // zlPanel
