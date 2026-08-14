// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "control_setting_panel.hpp"

namespace zlpanel {
    ControlSettingPanel::ControlSettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        base_(base), name_laf_(base),
        sensitivity_sliders_{
            {
                zlgui::slider::CompactLinearSlider<true, true, true>("Rough", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Fine", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Rough", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Fine", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Rough", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Fine", base),
                zlgui::slider::CompactLinearSlider<true, true, true>("Menu", base)
            }
        },
        wheel_reverse_box_(zlstate::PWheelShiftReverse::kChoices, base),
        rotary_style_box_(zlstate::PRotaryStyle::kChoices, base),
        rotary_drag_sensitivity_slider_("Distance", base),
        slider_double_click_box_(zlstate::PSliderDoubleClickFunc::kChoices, base),
        action_mouse_boxes_{
            zlgui::combobox::CompactCombobox(zlstate::PEnterSoloMouse::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PExitSoloMouse::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PRightClickMenuMouse::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PToggleDynamicMouse::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PToggleBypassMouse::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PDeleteBandMouse::kChoices, base)
        },
        action_key_boxes_{
            zlgui::combobox::CompactCombobox(zlstate::PEnterSoloKey::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PExitSoloKey::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PRightClickMenuKey::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PToggleDynamicKey::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PToggleBypassKey::kChoices, base),
            zlgui::combobox::CompactCombobox(zlstate::PDeleteBandKey::kChoices, base)
        } {
        juce::ignoreUnused(p_ref_);
        name_laf_.setFontScale(zlgui::kFontHuge);

        wheel_label_.setText("Wheel Sensitivity", juce::dontSendNotification);
        wheel_label_.setJustificationType(juce::Justification::centredRight);
        wheel_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(wheel_label_);
        slider_label_.setText("Slider Sensitivity", juce::dontSendNotification);
        slider_label_.setJustificationType(juce::Justification::centredRight);
        slider_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(slider_label_);
        dragger_label_.setText("Dragger Sensitivity", juce::dontSendNotification);
        dragger_label_.setJustificationType(juce::Justification::centredRight);
        dragger_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(dragger_label_);
        for (auto& s : sensitivity_sliders_) {
            s.getSlider().setRange(0.01, 1.0, 0.01);
            addAndMakeVisible(s);
        }
        addAndMakeVisible(wheel_reverse_box_);
        sensitivity_sliders_[0].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivity_sliders_[1].getSlider().setDoubleClickReturnValue(true, 0.12);
        sensitivity_sliders_[2].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivity_sliders_[3].getSlider().setDoubleClickReturnValue(true, 0.25);
        sensitivity_sliders_[4].getSlider().setDoubleClickReturnValue(true, 1.0);
        sensitivity_sliders_[5].getSlider().setDoubleClickReturnValue(true, 0.25);
        sensitivity_sliders_[6].getSlider().setDoubleClickReturnValue(true, 0.5);
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

        action_labels_[0].setText("Enter Solo", juce::dontSendNotification);
        action_labels_[1].setText("Exit Solo", juce::dontSendNotification);
        action_labels_[2].setText("Context Menu", juce::dontSendNotification);
        action_labels_[3].setText("Toggle Dynamic", juce::dontSendNotification);
        action_labels_[4].setText("Toggle Bypass", juce::dontSendNotification);
        action_labels_[5].setText("Delete Band", juce::dontSendNotification);

        for (size_t i = 0; i < 6; ++i) {
            action_labels_[i].setJustificationType(juce::Justification::centredRight);
            action_labels_[i].setLookAndFeel(&name_laf_);
            addAndMakeVisible(action_labels_[i]);
            addAndMakeVisible(action_mouse_boxes_[i]);
            addAndMakeVisible(action_key_boxes_[i]);
        }
    }

    ControlSettingPanel::~ControlSettingPanel() = default;

    void ControlSettingPanel::loadSetting() {
        for (size_t i = 0; i < sensitivity_sliders_.size(); ++i) {
            sensitivity_sliders_[i].getSlider().setValue(static_cast<double>(base_.getSensitivity(
                static_cast<zlgui::SensitivityIdx>(i))));
        }
        wheel_reverse_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getIsMouseWheelShiftReverse()));
        rotary_style_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getRotaryStyleID()));
        rotary_drag_sensitivity_slider_.getSlider().setValue(static_cast<double>(base_.getRotaryDragSensitivity()));
        slider_double_click_box_.getBox().setSelectedItemIndex(
            static_cast<int>(base_.getIsSliderDoubleClickOpenEditor()));
        action_mouse_boxes_[0].getBox().setSelectedItemIndex(static_cast<int>(base_.getEnterSoloMouse()));
        action_key_boxes_[0].getBox().setSelectedItemIndex(static_cast<int>(base_.getEnterSoloKey()));
        action_mouse_boxes_[1].getBox().setSelectedItemIndex(static_cast<int>(base_.getExitSoloMouse()));
        action_key_boxes_[1].getBox().setSelectedItemIndex(static_cast<int>(base_.getExitSoloKey()));
        action_mouse_boxes_[2].getBox().setSelectedItemIndex(static_cast<int>(base_.getContextMenuMouse()));
        action_key_boxes_[2].getBox().setSelectedItemIndex(static_cast<int>(base_.getContextMenuKey()));
        action_mouse_boxes_[3].getBox().setSelectedItemIndex(static_cast<int>(base_.getToggleDynamicMouse()));
        action_key_boxes_[3].getBox().setSelectedItemIndex(static_cast<int>(base_.getToggleDynamicKey()));
        action_mouse_boxes_[4].getBox().setSelectedItemIndex(static_cast<int>(base_.getToggleBypassMouse()));
        action_key_boxes_[4].getBox().setSelectedItemIndex(static_cast<int>(base_.getToggleBypassKey()));
        action_mouse_boxes_[5].getBox().setSelectedItemIndex(static_cast<int>(base_.getDeleteBandMouse()));
        action_key_boxes_[5].getBox().setSelectedItemIndex(static_cast<int>(base_.getDeleteBandKey()));
    }

    void ControlSettingPanel::saveSetting() {
        for (size_t i = 0; i < sensitivity_sliders_.size(); ++i) {
            base_.setSensitivity(static_cast<float>(sensitivity_sliders_[i].getSlider().getValue()),
                                 static_cast<zlgui::SensitivityIdx>(i));
        }
        base_.setIsMouseWheelShiftReverse(static_cast<bool>(wheel_reverse_box_.getBox().getSelectedItemIndex()));
        base_.setRotaryStyleID(static_cast<size_t>(rotary_style_box_.getBox().getSelectedItemIndex()));
        base_.setRotaryDragSensitivity(static_cast<float>(rotary_drag_sensitivity_slider_.getSlider().getValue()));
        base_.setIsSliderDoubleClickOpenEditor(
            static_cast<bool>(slider_double_click_box_.getBox().getSelectedItemIndex()));
        base_.setEnterSoloMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[0].getBox().getSelectedItemIndex()));
        base_.setEnterSoloKey(static_cast<zlgui::KeyActionType>(action_key_boxes_[0].getBox().getSelectedItemIndex()));
        base_.setExitSoloMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[1].getBox().getSelectedItemIndex()));
        base_.setExitSoloKey(static_cast<zlgui::KeyActionType>(action_key_boxes_[1].getBox().getSelectedItemIndex()));
        base_.setContextMenuMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[2].getBox().getSelectedItemIndex()));
        base_.setContextMenuKey(
            static_cast<zlgui::KeyActionType>(action_key_boxes_[2].getBox().getSelectedItemIndex()));
        base_.setToggleDynamicMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[3].getBox().getSelectedItemIndex()));
        base_.setToggleDynamicKey(
            static_cast<zlgui::KeyActionType>(action_key_boxes_[3].getBox().getSelectedItemIndex()));
        base_.setToggleBypassMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[4].getBox().getSelectedItemIndex()));
        base_.setToggleBypassKey(
            static_cast<zlgui::KeyActionType>(action_key_boxes_[4].getBox().getSelectedItemIndex()));
        base_.setDeleteBandMouse(
            static_cast<zlgui::MouseActionType>(action_mouse_boxes_[5].getBox().getSelectedItemIndex()));
        base_.setDeleteBandKey(static_cast<zlgui::KeyActionType>(action_key_boxes_[5].getBox().getSelectedItemIndex()));
        base_.saveToAPVTS();
    }

    void ControlSettingPanel::resetSetting() {
    }

    int ControlSettingPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return padding * 12 + slider_height * 11;
    }

    void ControlSettingPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderWidthScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);
        static constexpr int kLabelWidth = 2;

        auto bound = getLocalBounds();
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            wheel_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[0].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[1].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[6].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            wheel_reverse_box_.setBounds(local_bound.removeFromLeft(slider_width + padding).reduced(0, padding / 3));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            slider_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[2].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[3].setBounds(local_bound.removeFromLeft(slider_width));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            dragger_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[4].setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            sensitivity_sliders_[5].setBounds(local_bound.removeFromLeft(slider_width));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            rotary_style_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            rotary_style_box_.setBounds(local_bound.removeFromLeft(2 * slider_width).reduced(0, padding / 3));
            local_bound.removeFromLeft(padding);
            rotary_drag_sensitivity_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            slider_double_click_label_.setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            slider_double_click_box_.setBounds(local_bound.removeFromLeft(slider_width * 2).reduced(0, padding / 3));
        }

        for (size_t i = 0; i < 6; ++i) {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            action_labels_[i].setBounds(local_bound.removeFromLeft(slider_width * kLabelWidth));
            local_bound.removeFromLeft(padding);
            action_mouse_boxes_[i].setBounds(
                local_bound.removeFromLeft(static_cast<int>(slider_width * 2)).reduced(0, padding / 3));
            local_bound.removeFromLeft(padding);
            action_key_boxes_[i].setBounds(
                local_bound.removeFromLeft(static_cast<int>(slider_width * 2)).reduced(0, padding / 3));
        }
    }
}
