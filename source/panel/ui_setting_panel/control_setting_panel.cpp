// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_setting_panel.hpp"

namespace zlPanel {
    ControlSettingPanel::ControlSettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
    : pRef(p),
      uiBase(base), nameLAF(base),
    sensitivitySliders{
                  {
                      zlInterface::CompactLinearSlider("Rough", base),
                      zlInterface::CompactLinearSlider("Fine", base),
                      zlInterface::CompactLinearSlider("Rough", base),
                      zlInterface::CompactLinearSlider("Fine", base)
                  }
    },
    wheelReverseBox("", zlState::wheelShiftReverse::choices, base),
    rotaryStyleBox("", zlState::rotaryStyle::choices, base),
    rotaryDragSensitivitySlider("Distance", base) {
        juce::ignoreUnused(pRef);
        nameLAF.setJustification(juce::Justification::centredRight);
        nameLAF.setFontScale(zlInterface::FontHuge);

        wheelLabel.setText("Mouse-Wheel Sensitivity", juce::dontSendNotification);
        wheelLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(wheelLabel);
        dragLabel.setText("Mouse-Drag Sensitivity", juce::dontSendNotification);
        dragLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(dragLabel);
        for (auto &s: sensitivitySliders) {
            s.getSlider().setRange(0.0, 1.0, 0.01);
            addAndMakeVisible(s);
        }
        addAndMakeVisible(wheelReverseBox);
        sensitivitySliders[0].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivitySliders[1].getSlider().setDoubleClickReturnValue(true, 0.12);
        sensitivitySliders[2].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivitySliders[3].getSlider().setDoubleClickReturnValue(true, 0.25);
        rotaryStyleLabel.setText("Rotary Slider Style", juce::dontSendNotification);
        rotaryStyleLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(rotaryStyleLabel);
        addAndMakeVisible(rotaryStyleBox);
        rotaryDragSensitivitySlider.getSlider().setRange(2.0, 32.0, 0.01);
        rotaryDragSensitivitySlider.getSlider().setDoubleClickReturnValue(true, 10.0);
        addAndMakeVisible(rotaryDragSensitivitySlider);
    }

    ControlSettingPanel::~ControlSettingPanel() {
        wheelLabel.setLookAndFeel(nullptr);
        rotaryStyleLabel.setLookAndFeel(nullptr);
    }

    void ControlSettingPanel::loadSetting() {
        for (size_t i = 0; i < sensitivitySliders.size(); ++i) {
            sensitivitySliders[i].getSlider().setValue(static_cast<double>(uiBase.getSensitivity(
                static_cast<zlInterface::sensitivityIdx>(i))));
        }
        wheelReverseBox.getBox().setSelectedId(static_cast<int>(uiBase.getIsMouseWheelShiftReverse()) + 1);
        rotaryStyleBox.getBox().setSelectedId(static_cast<int>(uiBase.getRotaryStyleID()) + 1);
        rotaryDragSensitivitySlider.getSlider().setValue(static_cast<double>(uiBase.getRotaryDragSensitivity()));
    }

    void ControlSettingPanel::saveSetting() {
        for (size_t i = 0; i < sensitivitySliders.size(); ++i) {
            uiBase.setSensitivity(static_cast<float>(sensitivitySliders[i].getSlider().getValue()),
                                  static_cast<zlInterface::sensitivityIdx>(i));
        }
        uiBase.setIsMouseWheelShiftReverse(static_cast<bool>(wheelReverseBox.getBox().getSelectedId() - 1));
        uiBase.setRotaryStyleID(static_cast<size_t>(rotaryStyleBox.getBox().getSelectedId() - 1));
        uiBase.setRotaryDragSensitivity(static_cast<float>(rotaryDragSensitivitySlider.getSlider().getValue()));
        uiBase.saveToAPVTS();
    }

    void ControlSettingPanel::resetSetting() {
    }

    void ControlSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            wheelLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            sensitivitySliders[0].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            sensitivitySliders[1].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            wheelReverseBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            dragLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            sensitivitySliders[2].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            sensitivitySliders[3].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            rotaryStyleLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            rotaryStyleBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            rotaryDragSensitivitySlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
    }
} // zlPanel