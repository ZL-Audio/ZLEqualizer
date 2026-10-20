// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "state_definitions.hpp"
#include "../gui/interface_definitions.hpp"

namespace zlgui {
    enum class MouseActionType {
        kLeftClick,
        kRightClick,
        kLeftDoubleClick,
        kRightDoubleClick,
    };

    enum class KeyActionType {
        kNone,
        kCmdCtrl,
        kShift,
        kAlt,
    };

    enum ColourIdx {
        kTextColour,
        kBackgroundColour,
        kShadowColour,
        kGlowColour,
        kGridColour,
        kPreColour,
        kPostColour,
        kSideColour,
        kCollisionColour,
        kColourNum
    };

    static constexpr std::array<std::string_view, kColourNum> kColourNames = zlstate::kColourNames;

    enum PanelSettingIdx {
        kUISettingPanel,
        kUISettingChanged,
        kCurveShouldTransparent,
        kOutputPanel,
        kAnalyzerPanel,
        kDynamicExtraPanel,
        kFFTFrozen,
        kMatchPanel,
        kSuggestedNumBand,
        kMaximumNumBand,
        kPresetBrowser,
        kPanelSettingNum
    };

    inline std::array kPanelSettingIdentifiers{
        juce::Identifier("ui_setting_panel"),
        juce::Identifier("ui_setting_changed"),
        juce::Identifier("curve_should_transparent"),
        juce::Identifier("output_panel"),
        juce::Identifier("analyzer_panel"),
        juce::Identifier("dynamic_extra_panel"),
        juce::Identifier("fft_frozen"),
        juce::Identifier("match_panel"),
        juce::Identifier("suggested_num_band"),
        juce::Identifier("maximum_num_band"),
        juce::Identifier("preset_browser")
    };

    inline juce::Identifier kSoloIdentifier("solo_whole_idx");

    static constexpr size_t kColorMap1Size = 10;
    static constexpr size_t kColorMap2Size = 6;

    inline std::array kColourMaps = {
        std::vector<juce::Colour>{
            juce::Colour(31, 118, 179),
            juce::Colour(254, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 39),
            juce::Colour(147, 102, 189),
            juce::Colour(140, 86, 74),
            juce::Colour(219, 81, 177),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 33),
            juce::Colour(23, 189, 206)
        },
        std::vector<juce::Colour>{
            juce::Colour(214, 126, 65),
            juce::Colour(6, 134, 247),
            juce::Colour(211, 95, 211),
            juce::Colour(23, 198, 197),
            juce::Colour(107, 152, 66),
            juce::Colour(107, 161, 172),
            juce::Colour(40, 148, 73),
            juce::Colour(128, 128, 128),
            juce::Colour(91, 90, 245),
            juce::Colour(232, 65, 48)
        },
        std::vector<juce::Colour>{
            juce::Colour(76, 114, 176),
            juce::Colour(85, 168, 104),
            juce::Colour(196, 78, 82),
            juce::Colour(129, 114, 178),
            juce::Colour(255, 196, 0),
            juce::Colour(100, 181, 205),
        },
        std::vector<juce::Colour>{
            juce::Colour(179, 141, 79),
            juce::Colour(170, 87, 151),
            juce::Colour(59, 177, 173),
            juce::Colour(126, 141, 77),
            juce::Colour(51, 70, 139),
            juce::Colour(155, 74, 50),
        },
        std::vector<juce::Colour>{
            juce::Colour(0, 63, 255),
            juce::Colour(3, 237, 58),
            juce::Colour(232, 0, 11),
            juce::Colour(138, 43, 226),
            juce::Colour(255, 196, 0),
            juce::Colour(0, 215, 255),
        },
        std::vector<juce::Colour>{
            juce::Colour(225, 162, 0),
            juce::Colour(253, 19, 198),
            juce::Colour(0, 231, 220),
            juce::Colour(114, 209, 26),
            juce::Colour(46, 105, 255),
            juce::Colour(255, 49, 9)
        }
    };

    class UIState {
    public:
        UIBase gui;

        explicit UIState(juce::AudioProcessorValueTreeState& apvts) :
            gui({kColourMaps.begin(), kColourMaps.end()}, zlstate::PColourMapIdx::kChoices),
            state(apvts) {
            loadFromAPVTS();
        }

        juce::Colour getColourByIdx(ColourIdx idx) const {
            const auto index = static_cast<size_t>(idx);
            return index < kThemeColourNum ? gui.getThemeColour(static_cast<ThemeColour>(index))
                                           : plugin_colours_[index - kThemeColourNum];
        }

        void setColourByIdx(const ColourIdx idx, const juce::Colour colour) {
            const auto index = static_cast<size_t>(idx);
            if (index < kThemeColourNum) {
                gui.setThemeColour(static_cast<ThemeColour>(index), colour);
            } else {
                plugin_colours_[index - kThemeColourNum] = colour;
            }
        }

        size_t getRefreshRateID() const {
            return refresh_rate_id_.load(std::memory_order::relaxed);
        }

        void setRefreshRateID(const size_t x) {
            refresh_rate_id_.store(x, std::memory_order::relaxed);
        }

        size_t getFFTQuality() const {
            return fft_quality_.load(std::memory_order::relaxed);
        }

        void setFFTQuality(const size_t x) {
            fft_quality_.store(x, std::memory_order::relaxed);
        }

        float getFFTExtraTilt() const {
            return fft_extra_tilt_.load(std::memory_order::relaxed);
        }

        void setFFTExtraTilt(const float x) {
            fft_extra_tilt_.store(x, std::memory_order::relaxed);
        }

        float getFFTExtraSpeed() const {
            return fft_extra_speed_.load(std::memory_order::relaxed);
        }

        void setFFTExtraSpeed(const float x) {
            fft_extra_speed_.store(x, std::memory_order::relaxed);
        }

        float getSingleEQCurveThickness() const {
            return single_eq_curve_thickness_.load(std::memory_order::relaxed);
        }

        void setSingleEQCurveThickness(const float x) {
            single_eq_curve_thickness_.store(x, std::memory_order::relaxed);
        }

        float getSumEQCurveThickness() const {
            return sum_eq_curve_thickness_.load(std::memory_order::relaxed);
        }

        void setSumEQCurveThickness(const float x) {
            sum_eq_curve_thickness_.store(x, std::memory_order::relaxed);
        }

        void loadFromAPVTS();

        void saveToAPVTS() const;

        void setIsEditorShowing(const bool x) { is_editor_showing_ = x; }

        bool getIsEditorShowing() const { return is_editor_showing_; }

        juce::ValueTree& getPanelValueTree() { return panel_value_tree_; }

        void setWindowSizeFix(const bool f) { window_size_fix_ = f; }

        bool getWindowSizeFix() const { return window_size_fix_; }

        void setCurveDBScales(const std::array<float, 3> scales) {
            curve_db_scales_[0].store(scales[0], std::memory_order::relaxed);
            curve_db_scales_[1].store(scales[1], std::memory_order::relaxed);
            curve_db_scales_[2].store(scales[2], std::memory_order::relaxed);
        }

        auto getCurveDBScale(const size_t idx) const {
            return curve_db_scales_[idx].load(std::memory_order::relaxed);
        }

        bool isPanelIdentifier(const PanelSettingIdx idx, const juce::Identifier& identifier) const {
            return identifier == kPanelSettingIdentifiers[static_cast<size_t>(idx)];
        }

        juce::var getPanelProperty(const PanelSettingIdx idx) const {
            return panel_value_tree_.getProperty(kPanelSettingIdentifiers[static_cast<size_t>(idx)]);
        }

        void setPanelProperty(const PanelSettingIdx idx, const juce::var& v) {
            panel_value_tree_.setProperty(kPanelSettingIdentifiers[idx], v, nullptr);
        }

        juce::ValueTree& getSoloWholeIdxTree() { return solo_whole_idx_tree_; }

        size_t getSoloWholeIdx() const {
            return static_cast<size_t>(static_cast<int>(solo_whole_idx_tree_.getProperty(kSoloIdentifier)));
        }

        void setSoloWholeIdx(const size_t idx) {
            solo_whole_idx_tree_.setProperty(kSoloIdentifier, static_cast<int>(idx), nullptr);
        }

        size_t getSelectedBand() const { return selected_band_; }

        void setSelectedBand(const size_t x) { selected_band_ = x; }

        juce::SelectedItemSet<size_t>& getSelectedBandSet() { return selected_band_set_; }

        bool checkMouseKeyMatch(MouseActionType action_type, const juce::ModifierKeys& mods,
                                MouseActionType mouse_option, KeyActionType key_option) const;

        bool isEnterSoloTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, enter_solo_mouse_, enter_solo_key_);
        }

        bool isExitSoloTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, exit_solo_mouse_, exit_solo_key_);
        }

        bool isRightClickTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, right_click_menu_mouse_, right_click_menu_key_);
        }

        bool isToggleDynamicTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, toggle_dynamic_mouse_, toggle_dynamic_key_);
        }

        bool isToggleBypassTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, toggle_bypass_mouse_, toggle_bypass_key_);
        }

        bool isDeleteBandTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, delete_band_mouse_, delete_band_key_);
        }

        MouseActionType getEnterSoloMouse() const {
            return enter_solo_mouse_;
        }

        void setEnterSoloMouse(const MouseActionType x) {
            enter_solo_mouse_ = x;
        }

        KeyActionType getEnterSoloKey() const {
            return enter_solo_key_;
        }

        void setEnterSoloKey(const KeyActionType x) {
            enter_solo_key_ = x;
        }

        MouseActionType getExitSoloMouse() const {
            return exit_solo_mouse_;
        }

        void setExitSoloMouse(const MouseActionType x) {
            exit_solo_mouse_ = x;
        }

        KeyActionType getExitSoloKey() const {
            return exit_solo_key_;
        }

        void setExitSoloKey(const KeyActionType x) {
            exit_solo_key_ = x;
        }

        MouseActionType getContextMenuMouse() const {
            return right_click_menu_mouse_;
        }

        void setContextMenuMouse(const MouseActionType x) {
            right_click_menu_mouse_ = x;
        }

        KeyActionType getContextMenuKey() const {
            return right_click_menu_key_;
        }

        void setContextMenuKey(const KeyActionType x) {
            right_click_menu_key_ = x;
        }

        MouseActionType getToggleDynamicMouse() const {
            return toggle_dynamic_mouse_;
        }

        void setToggleDynamicMouse(const MouseActionType x) {
            toggle_dynamic_mouse_ = x;
        }

        KeyActionType getToggleDynamicKey() const {
            return toggle_dynamic_key_;
        }

        void setToggleDynamicKey(const KeyActionType x) {
            toggle_dynamic_key_ = x;
        }

        MouseActionType getToggleBypassMouse() const {
            return toggle_bypass_mouse_;
        }

        void setToggleBypassMouse(const MouseActionType x) {
            toggle_bypass_mouse_ = x;
        }

        KeyActionType getToggleBypassKey() const {
            return toggle_bypass_key_;
        }

        void setToggleBypassKey(const KeyActionType x) {
            toggle_bypass_key_ = x;
        }

        MouseActionType getDeleteBandMouse() const {
            return delete_band_mouse_;
        }

        void setDeleteBandMouse(const MouseActionType x) {
            delete_band_mouse_ = x;
        }

        KeyActionType getDeleteBandKey() const {
            return delete_band_key_;
        }

        void setDeleteBandKey(const KeyActionType x) {
            delete_band_key_ = x;
        }

    private:
        juce::AudioProcessorValueTreeState& state;
        juce::ValueTree panel_value_tree_{"panel_setting_tree"};
        juce::ValueTree solo_whole_idx_tree_{"solo_whole_idx_tree"};

        bool window_size_fix_{false};
        static constexpr size_t kThemeColourNum = static_cast<size_t>(ThemeColour::kCount);
        static_assert(kTextColour == 0 && kBackgroundColour == 1 && kShadowColour == 2 && kGlowColour == 3);
        std::array<juce::Colour, kColourNum - kThemeColourNum> plugin_colours_;
        std::atomic<size_t> refresh_rate_id_{2};
        std::atomic<size_t> fft_quality_{zlstate::PFFTQuality::kDefaultI};
        std::atomic<float> fft_extra_tilt_{0.f}, fft_extra_speed_{1.f};
        std::atomic<float> single_eq_curve_thickness_{1.f}, sum_eq_curve_thickness_{1.f};

        bool is_editor_showing_{false};

        std::array<std::atomic<float>, 3> curve_db_scales_{6.f, 12.f, 30.f};

        size_t selected_band_{zlstate::kBandNum};
        juce::SelectedItemSet<size_t> selected_band_set_;

        MouseActionType enter_solo_mouse_{MouseActionType::kRightClick};
        KeyActionType enter_solo_key_{KeyActionType::kNone};
        MouseActionType exit_solo_mouse_{MouseActionType::kRightClick};
        KeyActionType exit_solo_key_{KeyActionType::kNone};
        MouseActionType right_click_menu_mouse_{MouseActionType::kRightClick};
        KeyActionType right_click_menu_key_{KeyActionType::kAlt};
        MouseActionType toggle_dynamic_mouse_{MouseActionType::kLeftDoubleClick};
        KeyActionType toggle_dynamic_key_{KeyActionType::kCmdCtrl};
        MouseActionType toggle_bypass_mouse_{MouseActionType::kLeftClick};
        KeyActionType toggle_bypass_key_{KeyActionType::kAlt};
        MouseActionType delete_band_mouse_{MouseActionType::kRightDoubleClick};
        KeyActionType delete_band_key_{KeyActionType::kAlt};

        float loadPara(const std::string& id) const {
            return state.getRawParameterValue(id)->load(std::memory_order::relaxed);
        }

        void savePara(const std::string& id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(para->convertTo0to1(x));
            para->endChangeGesture();
        }
    };
}
