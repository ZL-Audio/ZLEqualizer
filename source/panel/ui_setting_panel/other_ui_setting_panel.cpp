// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlPanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p),
          uiBase(base), nameLAF(base),
          refreshRateBox("", zlState::refreshRate::choices, base),
          fftTiltSlider("Tilt", base),
          fftSpeedSlider("Speed", base),
          singleCurveSlider("Single", base),
          sumCurveSlider("Sum", base),
          defaultPassFilterSlopeBox("", zlState::defaultPassFilterSlope::choices, base) {
        juce::ignoreUnused(pRef);
        nameLAF.setJustification(juce::Justification::centredRight);
        nameLAF.setFontScale(zlInterface::FontHuge);
        refreshRateLabel.setText("Refresh Rate", juce::dontSendNotification);
        refreshRateLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(refreshRateLabel);
        addAndMakeVisible(refreshRateBox);
        fftLabel.setText("FFT", juce::dontSendNotification);
        fftLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(fftLabel);
        fftTiltSlider.getSlider().setNormalisableRange(zlState::fftExtraTilt::doubleRange);
        fftTiltSlider.getSlider().setDoubleClickReturnValue(true, static_cast<double>(zlState::fftExtraTilt::defaultV));
        fftSpeedSlider.getSlider().setNormalisableRange(zlState::fftExtraSpeed::doubleRange);
        fftSpeedSlider.getSlider().setDoubleClickReturnValue(
            true, static_cast<double>(zlState::fftExtraSpeed::defaultV));
        addAndMakeVisible(fftTiltSlider);
        addAndMakeVisible(fftSpeedSlider);
        curveThickLabel.setText("Curve Thickness", juce::dontSendNotification);
        curveThickLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(curveThickLabel);
        singleCurveSlider.getSlider().setNormalisableRange(zlState::singleCurveThickness::doubleRange);
        singleCurveSlider.getSlider().setDoubleClickReturnValue(true, zlState::singleCurveThickness::defaultV);
        sumCurveSlider.getSlider().setNormalisableRange(zlState::sumCurveThickness::doubleRange);
        sumCurveSlider.getSlider().setDoubleClickReturnValue(true, zlState::sumCurveThickness::defaultV);
        addAndMakeVisible(singleCurveSlider);
        addAndMakeVisible(sumCurveSlider);
        defaultPassFilterSlopeLabel.setText("Default Pass Filter Slope", juce::dontSendNotification);
        defaultPassFilterSlopeLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(defaultPassFilterSlopeLabel);
        addAndMakeVisible(defaultPassFilterSlopeBox);
    }

    OtherUISettingPanel::~OtherUISettingPanel() {
        refreshRateLabel.setLookAndFeel(nullptr);
        fftLabel.setLookAndFeel(nullptr);
        curveThickLabel.setLookAndFeel(nullptr);
        defaultPassFilterSlopeLabel.setLookAndFeel(nullptr);
    }

    void OtherUISettingPanel::loadSetting() {
        refreshRateBox.getBox().setSelectedId(static_cast<int>(uiBase.getRefreshRateID()) + 1);
        fftTiltSlider.getSlider().setValue(static_cast<double>(uiBase.getFFTExtraTilt()));
        fftSpeedSlider.getSlider().setValue(static_cast<double>(uiBase.getFFTExtraSpeed()));
        singleCurveSlider.getSlider().setValue(uiBase.getSingleCurveThickness());
        sumCurveSlider.getSlider().setValue(uiBase.getSumCurveThickness());
        defaultPassFilterSlopeBox.getBox().setSelectedId(uiBase.getDefaultPassFilterSlope() + 1);
    }

    void OtherUISettingPanel::saveSetting() {
        uiBase.setRefreshRateID(static_cast<size_t>(refreshRateBox.getBox().getSelectedId() - 1));
        uiBase.setFFTExtraTilt(static_cast<float>(fftTiltSlider.getSlider().getValue()));
        uiBase.setFFTExtraSpeed(static_cast<float>(fftSpeedSlider.getSlider().getValue()));
        uiBase.setSingleCurveThickness(static_cast<float>(singleCurveSlider.getSlider().getValue()));
        uiBase.setSumCurveThickness(static_cast<float>(sumCurveSlider.getSlider().getValue()));
        uiBase.setDefaultPassFilterSlope(defaultPassFilterSlopeBox.getBox().getSelectedId() - 1);
        uiBase.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    void OtherUISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
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
        }
    }
} // zlPanel
