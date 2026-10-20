// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_state.hpp"

namespace zlgui {
    void UIState::loadFromAPVTS() {
        for (size_t i = 0; i < kColourNum; ++i) {
            const auto r = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_r")));
            const auto g = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_g")));
            const auto b = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_b")));
            const auto o = loadPara(std::string(kColourNames[i]) + "_o");
            setColourByIdx(static_cast<ColourIdx>(i), juce::Colour(r, g, b, o));
        }
        gui.setFontMode(static_cast<size_t>(std::round(loadPara(zlstate::PFontMode::kID))));
        gui.setFontScale(loadPara(zlstate::PFontScale::kID));
        gui.setStaticFontSize(loadPara(zlstate::PStaticFontSize::kID));
        window_size_fix_ = loadPara(zlstate::PWindowSizeFix::kID) > .5f;
        gui.setSensitivity(loadPara(zlstate::PWheelSensitivity::kID), kMouseWheel);
        gui.setSensitivity(loadPara(zlstate::PWheelFineSensitivity::kID), kMouseWheelFine);
        gui.setSensitivity(loadPara(zlstate::PSliderSensitivity::kID), kMouseSlider);
        gui.setSensitivity(loadPara(zlstate::PSliderFineSensitivity::kID), kMouseSliderFine);
        gui.setSensitivity(loadPara(zlstate::PDraggerSensitivity::kID), kMouseDragger);
        gui.setSensitivity(loadPara(zlstate::PDraggerFineSensitivity::kID), kMouseDraggerFine);
        gui.setSensitivity(loadPara(zlstate::PWheelComboboxSensitivity::kID), kMouseWheelCombobox);
        gui.setIsMouseWheelShiftReverse(loadPara(zlstate::PWheelShiftReverse::kID) > .5f);
        gui.setRotaryStyleID(static_cast<size_t>(std::round(loadPara(zlstate::PRotaryStyle::kID))));
        gui.setRotaryDragSensitivity(loadPara(zlstate::PRotaryDragSensitivity::kID));
        gui.setIsSliderDoubleClickOpenEditor(loadPara(zlstate::PSliderDoubleClickFunc::kID) > .5f);
        refresh_rate_id_.store(static_cast<size_t>(std::round(loadPara(zlstate::PTargetRefreshSpeed::kID))));
        fft_quality_.store(static_cast<size_t>(std::round(loadPara(zlstate::PFFTQuality::kID))));
        fft_extra_tilt_.store(loadPara(zlstate::PFFTExtraTilt::kID));
        fft_extra_speed_.store(loadPara(zlstate::PFFTExtraSpeed::kID));
        gui.setComboboxAlignment(static_cast<size_t>(std::round(loadPara(zlstate::PComboboxAlignment::kID))));
        single_eq_curve_thickness_.store(loadPara(zlstate::PSingleEQCurveThickness::kID));
        sum_eq_curve_thickness_.store(loadPara(zlstate::PSumEQCurveThickness::kID));
        gui.setTooltipLandID(static_cast<size_t>(std::round(loadPara(zlstate::PTooltipLang::kID))));
        gui.setCMap1Idx(static_cast<size_t>(std::round(loadPara(zlstate::PColourMap1Idx::kID))));
        gui.setCMap2Idx(static_cast<size_t>(std::round(loadPara(zlstate::PColourMap2Idx::kID))));
        curve_db_scales_[0].store(loadPara(zlstate::PCurveDBScale0::kID));
        curve_db_scales_[1].store(loadPara(zlstate::PCurveDBScale1::kID));
        curve_db_scales_[2].store(loadPara(zlstate::PCurveDBScale2::kID));

        enter_solo_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PEnterSoloMouse::kID)));
        enter_solo_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PEnterSoloKey::kID)));
        exit_solo_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PExitSoloMouse::kID)));
        exit_solo_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PExitSoloKey::kID)));
        right_click_menu_mouse_ = static_cast<MouseActionType>(
            std::round(loadPara(zlstate::PRightClickMenuMouse::kID)));
        right_click_menu_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PRightClickMenuKey::kID)));
        toggle_dynamic_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PToggleDynamicMouse::kID)));
        toggle_dynamic_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PToggleDynamicKey::kID)));
        toggle_bypass_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PToggleBypassMouse::kID)));
        toggle_bypass_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PToggleBypassKey::kID)));
        delete_band_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PDeleteBandMouse::kID)));
        delete_band_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PDeleteBandKey::kID)));
    }

    void UIState::saveToAPVTS() const {
        for (size_t i = 0; i < kColourNum; ++i) {
            const auto colour = getColourByIdx(static_cast<ColourIdx>(i));
            const std::array<float, 4> rgbo = {
                static_cast<float>(colour.getRed()),
                static_cast<float>(colour.getGreen()),
                static_cast<float>(colour.getBlue()),
                colour.getFloatAlpha()
            };
            const std::array<std::string, 4> ID{
                std::string(kColourNames[i]) + "_r",
                std::string(kColourNames[i]) + "_g",
                std::string(kColourNames[i]) + "_b",
                std::string(kColourNames[i]) + "_o"
            };
            for (size_t j = 0; j < 4; ++j) {
                savePara(ID[j], rgbo[j]);
            }
        }
        savePara(zlstate::PFontMode::kID, static_cast<float>(gui.getFontMode()));
        savePara(zlstate::PFontScale::kID, gui.getFontScale());
        savePara(zlstate::PStaticFontSize::kID, gui.getStaticFontSize());
        savePara(zlstate::PWindowSizeFix::kID, window_size_fix_);
        savePara(zlstate::PWheelSensitivity::kID, gui.getSensitivity(kMouseWheel));
        savePara(zlstate::PWheelFineSensitivity::kID, gui.getSensitivity(kMouseWheelFine));
        savePara(zlstate::PSliderSensitivity::kID, gui.getSensitivity(kMouseSlider));
        savePara(zlstate::PSliderFineSensitivity::kID, gui.getSensitivity(kMouseSliderFine));
        savePara(zlstate::PDraggerSensitivity::kID, gui.getSensitivity(kMouseDragger));
        savePara(zlstate::PDraggerFineSensitivity::kID, gui.getSensitivity(kMouseDraggerFine));
        savePara(zlstate::PWheelComboboxSensitivity::kID, gui.getSensitivity(kMouseWheelCombobox));
        savePara(zlstate::PWheelShiftReverse::kID,
                 static_cast<float>(gui.getIsMouseWheelShiftReverse()));
        savePara(zlstate::PRotaryStyle::kID, static_cast<float>(gui.getRotaryStyleID()));
        savePara(zlstate::PRotaryDragSensitivity::kID, gui.getRotaryDragSensitivity());
        savePara(zlstate::PSliderDoubleClickFunc::kID,
                 static_cast<float>(gui.getIsSliderDoubleClickOpenEditor()));
        savePara(zlstate::PTargetRefreshSpeed::kID,
                 static_cast<float>(refresh_rate_id_.load(std::memory_order::relaxed)));
        savePara(zlstate::PFFTQuality::kID, static_cast<float>(fft_quality_.load(std::memory_order::relaxed)));
        savePara(zlstate::PFFTExtraTilt::kID, fft_extra_tilt_.load(std::memory_order::relaxed));
        savePara(zlstate::PFFTExtraSpeed::kID, fft_extra_speed_.load(std::memory_order::relaxed));
        savePara(zlstate::PComboboxAlignment::kID,
                 static_cast<float>(gui.getComboboxAlignment()));
        savePara(zlstate::PSingleEQCurveThickness::kID, single_eq_curve_thickness_.load(std::memory_order::relaxed));
        savePara(zlstate::PSumEQCurveThickness::kID, sum_eq_curve_thickness_.load(std::memory_order::relaxed));
        savePara(zlstate::PTooltipLang::kID, static_cast<float>(gui.getTooltipLangID()));
        savePara(zlstate::PColourMap1Idx::kID, static_cast<float>(gui.getCMap1Idx()));
        savePara(zlstate::PColourMap2Idx::kID, static_cast<float>(gui.getCMap2Idx()));
        savePara(zlstate::PCurveDBScale0::kID, curve_db_scales_[0].load(std::memory_order::relaxed));
        savePara(zlstate::PCurveDBScale1::kID, curve_db_scales_[1].load(std::memory_order::relaxed));
        savePara(zlstate::PCurveDBScale2::kID, curve_db_scales_[2].load(std::memory_order::relaxed));

        savePara(zlstate::PEnterSoloMouse::kID, static_cast<float>(enter_solo_mouse_));
        savePara(zlstate::PEnterSoloKey::kID, static_cast<float>(enter_solo_key_));
        savePara(zlstate::PExitSoloMouse::kID, static_cast<float>(exit_solo_mouse_));
        savePara(zlstate::PExitSoloKey::kID, static_cast<float>(exit_solo_key_));
        savePara(zlstate::PRightClickMenuMouse::kID, static_cast<float>(right_click_menu_mouse_));
        savePara(zlstate::PRightClickMenuKey::kID, static_cast<float>(right_click_menu_key_));
        savePara(zlstate::PToggleDynamicMouse::kID, static_cast<float>(toggle_dynamic_mouse_));
        savePara(zlstate::PToggleDynamicKey::kID, static_cast<float>(toggle_dynamic_key_));
        savePara(zlstate::PToggleBypassMouse::kID, static_cast<float>(toggle_bypass_mouse_));
        savePara(zlstate::PToggleBypassKey::kID, static_cast<float>(toggle_bypass_key_));
        savePara(zlstate::PDeleteBandMouse::kID, static_cast<float>(delete_band_mouse_));
        savePara(zlstate::PDeleteBandKey::kID, static_cast<float>(delete_band_key_));
    }

    bool UIState::checkMouseKeyMatch(const MouseActionType action_type, const juce::ModifierKeys& mods,
                                     const MouseActionType mouse_option, const KeyActionType key_option) const {
        bool key_match{false};
        switch (key_option) {
        case KeyActionType::kNone: {
            key_match = !mods.isCommandDown() && !mods.isShiftDown() && !mods.isAltDown();
            break;
        }
        case KeyActionType::kCmdCtrl: {
            key_match = mods.isCommandDown();
            break;
        }
        case KeyActionType::kShift: {
            key_match = mods.isShiftDown();
            break;
        }
        case KeyActionType::kAlt: {
            key_match = mods.isAltDown();
            break;
        }
        }
        if (!key_match) {
            return false;
        }

        return action_type == mouse_option;
    }
}
