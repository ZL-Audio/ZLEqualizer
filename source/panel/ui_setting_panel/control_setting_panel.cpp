// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "control_setting_panel.hpp"

namespace zlpanel {
    ControlSettingPanel::ControlSettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p),
          base_(base), name_laf_(base),
          sensitivity_sliders_{
              {
                  zlgui::slider::CompactLinearSlider<true, true, true>("Rough", base),
                  zlgui::slider::CompactLinearSlider<true, true, true>("Fine", base),
                  zlgui::slider::CompactLinearSlider<true, true, true>("Rough", base),
                  zlgui::slider::CompactLinearSlider<true, true, true>("Fine", base)
              }
          },
          wheel_reverse_box_(zlstate::PWheelShiftReverse::kChoices, base),
          rotary_style_box_(zlstate::PRotaryStyle::kChoices, base),
          rotary_drag_sensitivity_slider_("Distance", base),
          slider_double_click_box_(zlstate::PSliderDoubleClickFunc::kChoices, base) {
        juce::ignoreUnused(p_ref_);
        name_laf_.setFontScale(zlgui::kFontHuge);

        wheel_label_.setText("Wheel Sensitivity", juce::dontSendNotification);
        wheel_label_.setJustificationType(juce::Justification::centredRight);
        wheel_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(wheel_label_);
        drag_label_.setText("Drag Sensitivity", juce::dontSendNotification);
        drag_label_.setJustificationType(juce::Justification::centredRight);
        drag_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(drag_label_);
        for (auto &s: sensitivity_sliders_) {
            s.getSlider().setRange(0.0, 1.0, 0.01);
            addAndMakeVisible(s);
        }
        addAndMakeVisible(wheel_reverse_box_);
        sensitivity_sliders_[0].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivity_sliders_[1].getSlider().setDoubleClickReturnValue(true, 0.12);
        sensitivity_sliders_[2].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivity_sliders_[3].getSlider().setDoubleClickReturnValue(true, 0.25);
        rotary_style_label_.setText("Rotary Slider Style", juce::dontSendNotification);
        rotary_style_label_.setJustificationType(juce::Justification::centredRight);
        rotary_style_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(rotary_style_label_);
        addAndMakeVisible(rotary_style_box_);
        rotary_drag_sensitivity_slider_.getSlider().setRange(2.0, 32.0, 0.01);
        rotary_drag_sensitivity_slider_.getSlider().setDoubleClickReturnValue(true, 10.0);
        addAndMakeVisible(rotary_drag_sensitivity_slider_);
        slider_double_click_label_.setText("Slider Double Click", juce::dontSendNotification);
        slider_double_click_label_.setJustificationType(juce::Justification::centredRight);
        slider_double_click_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(slider_double_click_label_);
        addAndMakeVisible(slider_double_click_box_);

        import_label_.setText("Import Controls", juce::dontSendNotification);
        import_label_.setJustificationType(juce::Justification::centred);
        import_label_.setLookAndFeel(&name_laf_);
        import_label_.addMouseListener(this, false);
        addAndMakeVisible(import_label_);
        export_label_.setText("Export Controls", juce::dontSendNotification);
        export_label_.setJustificationType(juce::Justification::centred);
        export_label_.setLookAndFeel(&name_laf_);
        export_label_.addMouseListener(this, false);
        addAndMakeVisible(export_label_);
    }

    ControlSettingPanel::~ControlSettingPanel() = default;

    void ControlSettingPanel::loadSetting() {
        for (size_t i = 0; i < sensitivity_sliders_.size(); ++i) {
            sensitivity_sliders_[i].getSlider().setValue(static_cast<double>(base_.getSensitivity(
                static_cast<zlgui::SensitivityIdx>(i))));
        }
        wheel_reverse_box_.getBox().setSelectedId(static_cast<int>(base_.getIsMouseWheelShiftReverse()) + 1);
        rotary_style_box_.getBox().setSelectedId(static_cast<int>(base_.getRotaryStyleID()) + 1);
        rotary_drag_sensitivity_slider_.getSlider().setValue(static_cast<double>(base_.getRotaryDragSensitivity()));
        slider_double_click_box_.getBox().setSelectedId(
            static_cast<int>(base_.getIsSliderDoubleClickOpenEditor()) + 1);
    }

    void ControlSettingPanel::saveSetting() {
        for (size_t i = 0; i < sensitivity_sliders_.size(); ++i) {
            base_.setSensitivity(static_cast<float>(sensitivity_sliders_[i].getSlider().getValue()),
                                 static_cast<zlgui::SensitivityIdx>(i));
        }
        base_.setIsMouseWheelShiftReverse(static_cast<bool>(wheel_reverse_box_.getBox().getSelectedId() - 1));
        base_.setRotaryStyleID(static_cast<size_t>(rotary_style_box_.getBox().getSelectedId() - 1));
        base_.setRotaryDragSensitivity(static_cast<float>(rotary_drag_sensitivity_slider_.getSlider().getValue()));
        base_.setIsSliderDoubleClickOpenEditor(
            static_cast<bool>(slider_double_click_box_.getBox().getSelectedId() - 1));
        base_.saveToAPVTS();
    }

    void ControlSettingPanel::resetSetting() {
    }

    int ControlSettingPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return padding * 6 + slider_height * 5;
    }

    void ControlSettingPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderWidthScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);
        static constexpr int kLabelWidth = 2;

        auto bound = getLocalBounds(); {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            wheel_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[0].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[1].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            wheel_reverse_box_.setBounds(local_bound.removeFromLeft(slider_width + padding).reduced(0, padding / 3));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            drag_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[2].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[3].setBounds(local_bound.removeFromLeft(slider_width));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            rotary_style_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            rotary_style_box_.setBounds(local_bound.removeFromLeft(2 * slider_width).reduced(0, padding / 3));
            local_bound.removeFromLeft(padding);
            rotary_drag_sensitivity_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            slider_double_click_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            slider_double_click_box_.setBounds(local_bound.removeFromLeft(slider_width * 2).reduced(0, padding / 3));
        } {
            bound.removeFromTop(padding);
            const auto label_width = bound.getWidth() / 2;
            auto local_bound = bound.removeFromTop(slider_height);
            import_label_.setBounds(local_bound.removeFromLeft(label_width));
            export_label_.setBounds(local_bound.removeFromRight(label_width));
        }
    }

    void ControlSettingPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent == &import_label_) {
            importControls();
        } else if (event.originalComponent == &export_label_) {
            exportControls();
        }
    }

    void ControlSettingPanel::importControls() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Load the control settings...", kSettingDirectory, "*.xml",
            true, false, nullptr);
        constexpr auto setting_open_flags = juce::FileBrowserComponent::openMode |
                                            juce::FileBrowserComponent::canSelectFiles;
        chooser_->launchAsync(setting_open_flags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (const auto xml_input = juce::XmlDocument::parse(settingFile)) {
                if (const auto *xml_element = xml_input->getChildByName("drag_fine_sensitivity")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseDragFine);
                }
                if (const auto *xml_element = xml_input->getChildByName("drag_sensitivity")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseDrag);
                }
                if (const auto *xml_element = xml_input->getChildByName("wheel_fine_sensitivity")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseWheelFine);
                }
                if (const auto *xml_element = xml_input->getChildByName("wheel_sensitivity")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setSensitivity(static_cast<float>(x), zlgui::SensitivityIdx::kMouseWheel);
                }
                if (const auto *xml_element = xml_input->getChildByName("rotary_drag_sensitivity")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setRotaryDragSensitivity(static_cast<float>(x));
                }
                if (const auto *xml_element = xml_input->getChildByName("rotary_style")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setRotaryStyleID(static_cast<size_t>(x));
                }
                if (const auto *xml_element = xml_input->getChildByName("slider_double_click_func")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setIsSliderDoubleClickOpenEditor(x > 0.5);
                }
                if (const auto *xml_element = xml_input->getChildByName("wheel_shift_reverse")) {
                    const auto x = xml_element->getDoubleAttribute("value");
                    base_.setIsMouseWheelShiftReverse(x > 0.5);
                }
                base_.saveToAPVTS();
                loadSetting();
            }
        });
    }

    void ControlSettingPanel::exportControls() {
        chooser_ = std::make_unique<juce::FileChooser>(
            "Save the control settings...", kSettingDirectory.getChildFile("control.xml"), "*.xml",
            true, false, nullptr);
        constexpr auto setting_save_flags = juce::FileBrowserComponent::saveMode |
                                            juce::FileBrowserComponent::warnAboutOverwriting;
        chooser_->launchAsync(setting_save_flags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            juce::File setting_file(chooser.getResult().withFileExtension("xml"));
            if (setting_file.create()) {
                saveSetting();
                juce::XmlElement xml_output{"colour_setting"}; {
                    auto *xml_element = xml_output.createNewChildElement("drag_fine_sensitivity");
                    xml_element->setAttribute("value", base_.getSensitivity(zlgui::kMouseDragFine));
                } {
                    auto *xml_element = xml_output.createNewChildElement("drag_sensitivity");
                    xml_element->setAttribute("value", base_.getSensitivity(zlgui::kMouseDrag));
                } {
                    auto *xml_element = xml_output.createNewChildElement("wheel_fine_sensitivity");
                    xml_element->setAttribute("value", base_.getSensitivity(zlgui::kMouseWheelFine));
                } {
                    auto *xml_element = xml_output.createNewChildElement("wheel_sensitivity");
                    xml_element->setAttribute("value", base_.getSensitivity(zlgui::kMouseWheel));
                } {
                    auto *xml_element = xml_output.createNewChildElement("rotary_drag_sensitivity");
                    xml_element->setAttribute("value", base_.getRotaryDragSensitivity());
                } {
                    auto *xml_element = xml_output.createNewChildElement("rotary_style");
                    xml_element->setAttribute("value", static_cast<double>(base_.getRotaryStyleID()));
                } {
                    auto *xml_element = xml_output.createNewChildElement("slider_double_click_func");
                    xml_element->
                            setAttribute("value", static_cast<double>(base_.getIsSliderDoubleClickOpenEditor()));
                } {
                    auto *xml_element = xml_output.createNewChildElement("wheel_shift_reverse");
                    xml_element->setAttribute("value", static_cast<double>(base_.getIsMouseWheelShiftReverse()));
                }
                const auto result = xml_output.writeTo(setting_file);
                juce::ignoreUnused(result);
            }
        });
    }
} // zlpanel
