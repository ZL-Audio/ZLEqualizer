// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_setting_panel.hpp"

namespace zlpanel {
    ControlSettingPanel::ControlSettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p),
          ui_base_(base), nameLAF(base),
          sensitivitySliders{
              {
                  zlgui::CompactLinearSlider("Rough", base),
                  zlgui::CompactLinearSlider("Fine", base),
                  zlgui::CompactLinearSlider("Rough", base),
                  zlgui::CompactLinearSlider("Fine", base)
              }
          },
          wheelReverseBox("", zlstate::wheelShiftReverse::choices, base),
          rotaryStyleBox("", zlstate::rotaryStyle::choices, base),
          rotaryDragSensitivitySlider("Distance", base),
          sliderDoubleClickBox("", zlstate::sliderDoubleClickFunc::choices, base) {
        juce::ignoreUnused(pRef);
        nameLAF.setFontScale(zlgui::kFontHuge);

        wheelLabel.setText("Mouse-Wheel Sensitivity", juce::dontSendNotification);
        wheelLabel.setJustificationType(juce::Justification::centredRight);
        wheelLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(wheelLabel);
        dragLabel.setText("Mouse-Drag Sensitivity", juce::dontSendNotification);
        dragLabel.setJustificationType(juce::Justification::centredRight);
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
        rotaryStyleLabel.setJustificationType(juce::Justification::centredRight);
        rotaryStyleLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(rotaryStyleLabel);
        addAndMakeVisible(rotaryStyleBox);
        rotaryDragSensitivitySlider.getSlider().setRange(2.0, 32.0, 0.01);
        rotaryDragSensitivitySlider.getSlider().setDoubleClickReturnValue(true, 10.0);
        addAndMakeVisible(rotaryDragSensitivitySlider);
        sliderDoubleClickLabel.setText("Slider Double Click", juce::dontSendNotification);
        sliderDoubleClickLabel.setJustificationType(juce::Justification::centredRight);
        sliderDoubleClickLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(sliderDoubleClickLabel);
        addAndMakeVisible(sliderDoubleClickBox);

        importLabel.setText("Import Controls", juce::dontSendNotification);
        importLabel.setJustificationType(juce::Justification::centred);
        importLabel.setLookAndFeel(&nameLAF);
        importLabel.addMouseListener(this, false);
        addAndMakeVisible(importLabel);
        exportLabel.setText("Export Controls", juce::dontSendNotification);
        exportLabel.setJustificationType(juce::Justification::centred);
        exportLabel.setLookAndFeel(&nameLAF);
        exportLabel.addMouseListener(this, false);
        addAndMakeVisible(exportLabel);
    }

    ControlSettingPanel::~ControlSettingPanel() = default;

    void ControlSettingPanel::loadSetting() {
        for (size_t i = 0; i < sensitivitySliders.size(); ++i) {
            sensitivitySliders[i].getSlider().setValue(static_cast<double>(ui_base_.getSensitivity(
                static_cast<zlgui::SensitivityIdx>(i))));
        }
        wheelReverseBox.getBox().setSelectedId(static_cast<int>(ui_base_.getIsMouseWheelShiftReverse()) + 1);
        rotaryStyleBox.getBox().setSelectedId(static_cast<int>(ui_base_.getRotaryStyleID()) + 1);
        rotaryDragSensitivitySlider.getSlider().setValue(static_cast<double>(ui_base_.getRotaryDragSensitivity()));
        sliderDoubleClickBox.getBox().setSelectedId(static_cast<int>(ui_base_.getIsSliderDoubleClickOpenEditor()) + 1);
    }

    void ControlSettingPanel::saveSetting() {
        for (size_t i = 0; i < sensitivitySliders.size(); ++i) {
            ui_base_.setSensitivity(static_cast<float>(sensitivitySliders[i].getSlider().getValue()),
                                  static_cast<zlgui::SensitivityIdx>(i));
        }
        ui_base_.setIsMouseWheelShiftReverse(static_cast<bool>(wheelReverseBox.getBox().getSelectedId() - 1));
        ui_base_.setRotaryStyleID(static_cast<size_t>(rotaryStyleBox.getBox().getSelectedId() - 1));
        ui_base_.setRotaryDragSensitivity(static_cast<float>(rotaryDragSensitivitySlider.getSlider().getValue()));
        ui_base_.setIsSliderDoubleClickOpenEditor(static_cast<bool>(sliderDoubleClickBox.getBox().getSelectedId() - 1));
        ui_base_.saveToAPVTS();
    }

    void ControlSettingPanel::resetSetting() {
    }

    void ControlSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            wheelLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            sensitivitySliders[0].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            sensitivitySliders[1].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            wheelReverseBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            dragLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            sensitivitySliders[2].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            sensitivitySliders[3].setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            rotaryStyleLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            rotaryStyleBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            rotaryDragSensitivitySlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
        {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            sliderDoubleClickLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .05f);
            const auto sWidth = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.425f;
            sliderDoubleClickBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto localBound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            importLabel.setBounds(localBound.removeFromLeft(bound.getWidth() * .45f).toNearestInt());
            localBound.removeFromLeft(bound.getWidth() * .10f);
            exportLabel.setBounds(localBound.toNearestInt());
        }
    }

    void ControlSettingPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent == &importLabel) {
            importControls();
        } else if (event.originalComponent == &exportLabel) {
            exportControls();
        }
    }

    void ControlSettingPanel::importControls() {
        myChooser = std::make_unique<juce::FileChooser>(
                "Load the control settings...", settingDirectory, "*.xml",
                true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (const auto xmlInput = juce::XmlDocument::parse(settingFile)) {
                if (const auto *xmlElement = xmlInput->getChildByName("drag_fine_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseDragFine);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("drag_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseDrag);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_fine_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseWheelFine);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseWheel);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("rotary_drag_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setRotaryDragSensitivity(static_cast<float>(x));
                }
                if (const auto *xmlElement = xmlInput->getChildByName("rotary_style")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setRotaryStyleID(static_cast<size_t>(x));
                }
                if (const auto *xmlElement = xmlInput->getChildByName("slider_double_click_func")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setIsSliderDoubleClickOpenEditor(x > 0.5);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_shift_reverse")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    ui_base_.setIsMouseWheelShiftReverse(x > 0.5);
                }
                ui_base_.saveToAPVTS();
                loadSetting();
            }
        });
    }

    void ControlSettingPanel::exportControls() {
        myChooser = std::make_unique<juce::FileChooser>(
                "Save the control settings...", settingDirectory.getChildFile("control.xml"), "*.xml",
                true, false, nullptr);
        constexpr auto settingSaveFlags = juce::FileBrowserComponent::saveMode |
                                          juce::FileBrowserComponent::warnAboutOverwriting;
        myChooser->launchAsync(settingSaveFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            juce::File settingFile(chooser.getResult().withFileExtension("xml"));
            if (settingFile.create()) {
                saveSetting();
                juce::XmlElement xmlOutput{"colour_setting"};
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("drag_fine_sensitivity");
                    xmlElement->setAttribute("value", ui_base_.getSensitivity(zlgui::kMouseDragFine));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("drag_sensitivity");
                    xmlElement->setAttribute("value", ui_base_.getSensitivity(zlgui::kMouseDrag));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("wheel_fine_sensitivity");
                    xmlElement->setAttribute("value", ui_base_.getSensitivity(zlgui::kMouseWheelFine));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("wheel_sensitivity");
                    xmlElement->setAttribute("value", ui_base_.getSensitivity(zlgui::kMouseWheel));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("rotary_drag_sensitivity");
                    xmlElement->setAttribute("value", ui_base_.getRotaryDragSensitivity());
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("rotary_style");
                    xmlElement->setAttribute("value", static_cast<double>(ui_base_.getRotaryStyleID()));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("slider_double_click_func");
                    xmlElement->setAttribute("value", static_cast<double>(ui_base_.getIsSliderDoubleClickOpenEditor()));
                }
                {
                    auto *xmlElement = xmlOutput.createNewChildElement("wheel_shift_reverse");
                    xmlElement->setAttribute("value", static_cast<double>(ui_base_.getIsMouseWheelShiftReverse()));
                }
                const auto result = xmlOutput.writeTo(settingFile);
                juce::ignoreUnused(result);
            }
        });
    }
} // zlpanel
