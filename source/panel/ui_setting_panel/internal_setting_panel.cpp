// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "internal_setting_panel.hpp"

namespace zlPanel {
    InternalSettingPanel::InternalSettingPanel(zlInterface::UIBase &base)
        : uiBase(base), nameLAF(base),
          preSelector(base, *this),
          postSelector(base, *this),
          sideSelector(base, *this),
          gridSelector(base, *this),
    roughWheelSlider("Rough", base),
    fineWheelSlider("Fine", base) {
        nameLAF.setJustification(juce::Justification::centredRight);
        nameLAF.setFontScale(zlInterface::FontHuge);
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setText(selectorNames[i], juce::dontSendNotification);
            selectorLabels[i].setLookAndFeel(&nameLAF);
            addAndMakeVisible(selectorLabels[i]);
            addAndMakeVisible(selectors[i]);
        }
        wheelLabel.setText("Mouse-Wheel Sensitivity", juce::dontSendNotification);
        wheelLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(wheelLabel);
        for (auto &s : {&roughWheelSlider, &fineWheelSlider}) {
            s->getSlider().setRange(0.0, 1.0, 0.01);
            addAndMakeVisible(s);
        }
    }

    InternalSettingPanel::~InternalSettingPanel() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setLookAndFeel(nullptr);
        }
        wheelLabel.setLookAndFeel(nullptr);
    }

    void InternalSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < numSelectors; ++i) {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            selectorLabels[i].setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            selectors[i]->setBounds(localBound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        }
        {
            bound.removeFromTop(uiBase.getFontSize());
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * 3);
            wheelLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - uiBase.getFontSize() * 2.f) * 0.3f;
            roughWheelSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 2.f);
            fineWheelSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
    }

    void InternalSettingPanel::loadSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectors[i]->setColour(uiBase.getColourByIdx(static_cast<zlInterface::colourIdx>(i)));
        }
        roughWheelSlider.getSlider().setValue(static_cast<double>(uiBase.getWheelSensitivity(0)));
        fineWheelSlider.getSlider().setValue(static_cast<double>(uiBase.getWheelSensitivity(1)));
    }

    void InternalSettingPanel::saveSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            uiBase.setColourByIdx(static_cast<zlInterface::colourIdx>(i), selectors[i]->getColour());
        }
        uiBase.setWheelSensitivity(static_cast<float>(roughWheelSlider.getSlider().getValue()), 0);
        uiBase.setWheelSensitivity(static_cast<float>(fineWheelSlider.getSlider().getValue()), 1);
        uiBase.saveToAPVTS();
    }

    void InternalSettingPanel::resetSetting() {
        loadSetting();
    }
} // zlPanel
