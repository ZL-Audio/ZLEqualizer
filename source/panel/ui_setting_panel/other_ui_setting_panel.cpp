// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlPanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p),
          uiBase(base), nameLAF(base),
          renderingEngineBox("", zlState::renderingEngine::choices, base),
          refreshRateBox("", zlState::refreshRate::choices, base),
          fftTiltSlider("Tilt", base),
          fftSpeedSlider("Speed", base),
          fftOrderBox("order", zlState::ffTOrder::choices, base),
          singleCurveSlider("Single", base),
          sumCurveSlider("Sum", base),
          defaultPassFilterSlopeBox("", zlState::defaultPassFilterSlope::choices, base),
          dynLinkBox("", zlState::dynLink::choices, base),
          tooltipONBox("", zlState::tooltipON::choices, base),
          tooltipLangBox("", zlState::tooltipLang::choices, base) {
        juce::ignoreUnused(pRef);
        nameLAF.setFontScale(zlInterface::FontHuge);
        renderingEngineLabel.setText("Rendering Engine", juce::dontSendNotification);
        renderingEngineLabel.setJustificationType(juce::Justification::centredRight);
        renderingEngineLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(renderingEngineLabel);
        addAndMakeVisible(renderingEngineBox);
        refreshRateLabel.setText("Refresh Rate", juce::dontSendNotification);
        refreshRateLabel.setJustificationType(juce::Justification::centredRight);
        refreshRateLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(refreshRateLabel);
        addAndMakeVisible(refreshRateBox);
        fftLabel.setText("FFT", juce::dontSendNotification);
        fftLabel.setJustificationType(juce::Justification::centredRight);
        fftLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(fftLabel);
        fftTiltSlider.getSlider().setNormalisableRange(zlState::fftExtraTilt::doubleRange);
        fftTiltSlider.getSlider().setDoubleClickReturnValue(true, static_cast<double>(zlState::fftExtraTilt::defaultV));
        fftSpeedSlider.getSlider().setNormalisableRange(zlState::fftExtraSpeed::doubleRange);
        fftSpeedSlider.getSlider().setDoubleClickReturnValue(
            true, static_cast<double>(zlState::fftExtraSpeed::defaultV));
        addAndMakeVisible(fftTiltSlider);
        addAndMakeVisible(fftSpeedSlider);
        addAndMakeVisible(fftOrderBox);
        curveThickLabel.setText("Curve Thickness", juce::dontSendNotification);
        curveThickLabel.setJustificationType(juce::Justification::centredRight);
        curveThickLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(curveThickLabel);
        singleCurveSlider.getSlider().setNormalisableRange(zlState::singleCurveThickness::doubleRange);
        singleCurveSlider.getSlider().setDoubleClickReturnValue(true, zlState::singleCurveThickness::defaultV);
        sumCurveSlider.getSlider().setNormalisableRange(zlState::sumCurveThickness::doubleRange);
        sumCurveSlider.getSlider().setDoubleClickReturnValue(true, zlState::sumCurveThickness::defaultV);
        addAndMakeVisible(singleCurveSlider);
        addAndMakeVisible(sumCurveSlider);
        defaultPassFilterSlopeLabel.setText("Default Pass Filter Slope", juce::dontSendNotification);
        defaultPassFilterSlopeLabel.setJustificationType(juce::Justification::centredRight);
        defaultPassFilterSlopeLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(defaultPassFilterSlopeLabel);
        addAndMakeVisible(defaultPassFilterSlopeBox);
        dynLinkLabel.setText("Default Dynamic Link", juce::dontSendNotification);
        dynLinkLabel.setJustificationType(juce::Justification::centredRight);
        dynLinkLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(dynLinkLabel);
        addAndMakeVisible(dynLinkBox);
        tooltipLabel.setText("Tooltip", juce::dontSendNotification);
        tooltipLabel.setJustificationType(juce::Justification::centredRight);
        tooltipLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(tooltipLabel);
        addAndMakeVisible(tooltipONBox);
        addAndMakeVisible(tooltipLangBox);
    }

    void OtherUISettingPanel::loadSetting() {
        renderingEngineBox.getBox().setSelectedId(static_cast<int>(uiBase.getRenderingEngine()) + 1);
        refreshRateBox.getBox().setSelectedId(static_cast<int>(uiBase.getRefreshRateID()) + 1);
        fftTiltSlider.getSlider().setValue(static_cast<double>(uiBase.getFFTExtraTilt()));
        fftSpeedSlider.getSlider().setValue(static_cast<double>(uiBase.getFFTExtraSpeed()));
        fftOrderBox.getBox().setSelectedId(uiBase.getFFTOrderIdx() + 1);
        singleCurveSlider.getSlider().setValue(uiBase.getSingleCurveThickness());
        sumCurveSlider.getSlider().setValue(uiBase.getSumCurveThickness());
        defaultPassFilterSlopeBox.getBox().setSelectedId(uiBase.getDefaultPassFilterSlope() + 1);
        dynLinkBox.getBox().setSelectedId(static_cast<int>(uiBase.getDynLink()) + 1);
        tooltipONBox.getBox().setSelectedId(static_cast<int>(uiBase.getTooltipON()) + 1);
        tooltipLangBox.getBox().setSelectedId(uiBase.getLangIdx() + 1);
    }

    void OtherUISettingPanel::saveSetting() {
        uiBase.setRenderingEngine(static_cast<int>(renderingEngineBox.getBox().getSelectedId() - 1));
        uiBase.setRefreshRateID(static_cast<size_t>(refreshRateBox.getBox().getSelectedId() - 1));
        uiBase.setFFTExtraTilt(static_cast<float>(fftTiltSlider.getSlider().getValue()));
        uiBase.setFFTExtraSpeed(static_cast<float>(fftSpeedSlider.getSlider().getValue()));
        uiBase.setFFTOrderIdx(fftOrderBox.getBox().getSelectedId() - 1);
        uiBase.setSingleCurveThickness(static_cast<float>(singleCurveSlider.getSlider().getValue()));
        uiBase.setSumCurveThickness(static_cast<float>(sumCurveSlider.getSlider().getValue()));
        uiBase.setDefaultPassFilterSlope(defaultPassFilterSlopeBox.getBox().getSelectedId() - 1);
        uiBase.setDynLink(dynLinkBox.getBox().getSelectedId() == 2);
        uiBase.setTooltipON(tooltipONBox.getBox().getSelectedId() == 2);
        uiBase.setLangIdx(tooltipLangBox.getBox().getSelectedId() - 1);
        uiBase.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    void OtherUISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            renderingEngineLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            renderingEngineBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            refreshRateLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            refreshRateBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            fftLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            fftTiltSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            fftSpeedSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            fftOrderBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            curveThickLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            singleCurveSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            sumCurveSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            defaultPassFilterSlopeLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            defaultPassFilterSlopeBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            dynLinkLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            dynLinkBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            tooltipLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            tooltipONBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            tooltipLangBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
    }
} // zlPanel
