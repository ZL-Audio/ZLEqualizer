// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p),
          ui_base_(base), nameLAF(base),
          renderingEngineBox("", zlstate::renderingEngine::choices, base),
          refreshRateBox("", zlstate::refreshRate::choices, base),
          fftTiltSlider("Tilt", base),
          fftSpeedSlider("Speed", base),
          fftOrderBox("order", zlstate::ffTOrder::choices, base),
          singleCurveSlider("Single", base),
          sumCurveSlider("Sum", base),
          defaultPassFilterSlopeBox("", zlstate::defaultPassFilterSlope::choices, base),
          dynLinkBox("", zlstate::dynLink::choices, base),
          tooltipONBox("", zlstate::tooltipON::choices, base),
          tooltipLangBox("", zlstate::tooltipLang::choices, base) {
        juce::ignoreUnused(pRef);
        nameLAF.setFontScale(zlgui::kFontHuge);
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
        fftTiltSlider.getSlider().setNormalisableRange(zlstate::fftExtraTilt::doubleRange);
        fftTiltSlider.getSlider().setDoubleClickReturnValue(true, static_cast<double>(zlstate::fftExtraTilt::defaultV));
        fftSpeedSlider.getSlider().setNormalisableRange(zlstate::fftExtraSpeed::doubleRange);
        fftSpeedSlider.getSlider().setDoubleClickReturnValue(
            true, static_cast<double>(zlstate::fftExtraSpeed::defaultV));
        addAndMakeVisible(fftTiltSlider);
        addAndMakeVisible(fftSpeedSlider);
        addAndMakeVisible(fftOrderBox);
        curveThickLabel.setText("Curve Thickness", juce::dontSendNotification);
        curveThickLabel.setJustificationType(juce::Justification::centredRight);
        curveThickLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(curveThickLabel);
        singleCurveSlider.getSlider().setNormalisableRange(zlstate::singleCurveThickness::doubleRange);
        singleCurveSlider.getSlider().setDoubleClickReturnValue(true, zlstate::singleCurveThickness::defaultV);
        sumCurveSlider.getSlider().setNormalisableRange(zlstate::sumCurveThickness::doubleRange);
        sumCurveSlider.getSlider().setDoubleClickReturnValue(true, zlstate::sumCurveThickness::defaultV);
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
        renderingEngineBox.getBox().setSelectedId(static_cast<int>(ui_base_.getRenderingEngine()) + 1);
        DBG(ui_base_.getRenderingEngine() + 1);
        refreshRateBox.getBox().setSelectedId(static_cast<int>(ui_base_.getRefreshRateID()) + 1);
        fftTiltSlider.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraTilt()));
        fftSpeedSlider.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraSpeed()));
        fftOrderBox.getBox().setSelectedId(ui_base_.getFFTOrderIdx() + 1);
        singleCurveSlider.getSlider().setValue(ui_base_.getSingleCurveThickness());
        sumCurveSlider.getSlider().setValue(ui_base_.getSumCurveThickness());
        defaultPassFilterSlopeBox.getBox().setSelectedId(ui_base_.getDefaultPassFilterSlope() + 1);
        dynLinkBox.getBox().setSelectedId(static_cast<int>(ui_base_.getDynLink()) + 1);
        tooltipONBox.getBox().setSelectedId(static_cast<int>(ui_base_.getTooltipON()) + 1);
        tooltipLangBox.getBox().setSelectedId(ui_base_.getLangIdx() + 1);
    }

    void OtherUISettingPanel::saveSetting() {
        ui_base_.setRenderingEngine(static_cast<int>(renderingEngineBox.getBox().getSelectedId() - 1));
        ui_base_.setRefreshRateID(static_cast<size_t>(refreshRateBox.getBox().getSelectedId() - 1));
        ui_base_.setFFTExtraTilt(static_cast<float>(fftTiltSlider.getSlider().getValue()));
        ui_base_.setFFTExtraSpeed(static_cast<float>(fftSpeedSlider.getSlider().getValue()));
        ui_base_.setFFTOrderIdx(fftOrderBox.getBox().getSelectedId() - 1);
        ui_base_.setSingleCurveThickness(static_cast<float>(singleCurveSlider.getSlider().getValue()));
        ui_base_.setSumCurveThickness(static_cast<float>(sumCurveSlider.getSlider().getValue()));
        ui_base_.setDefaultPassFilterSlope(defaultPassFilterSlopeBox.getBox().getSelectedId() - 1);
        ui_base_.setDynLink(dynLinkBox.getBox().getSelectedId() == 2);
        ui_base_.setTooltipON(tooltipONBox.getBox().getSelectedId() == 2);
        ui_base_.setLangIdx(tooltipLangBox.getBox().getSelectedId() - 1);
        ui_base_.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    void OtherUISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            renderingEngineLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.6f;
            renderingEngineBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            refreshRateLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            refreshRateBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            fftLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            fftTiltSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            fftSpeedSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            fftOrderBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            curveThickLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            singleCurveSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            sumCurveSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            defaultPassFilterSlopeLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            defaultPassFilterSlopeBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            dynLinkLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            dynLinkBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            tooltipLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            tooltipONBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 3.f);
            tooltipLangBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
    }

    void OtherUISettingPanel::setRendererList(const juce::StringArray &rendererList) {
        auto idx = renderingEngineBox.getBox().getSelectedItemIndex();
        renderingEngineBox.getBox().clear();
        renderingEngineBox.getBox().addItemList(rendererList, 1);
        if (idx >= 0) {
            idx = std::min(idx, renderingEngineBox.getBox().getNumItems() - 1);
            renderingEngineBox.getBox().setSelectedItemIndex(idx);
        }
    }
} // zlpanel
