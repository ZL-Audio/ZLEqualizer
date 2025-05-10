// Copyright (C) 2025 - zsliu98
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

#include "../state/state_definitions.hpp"
#include "multilingual/multilingual.hpp"

namespace zlgui {
    enum ColourIdx {
        kTextColour,
        kBackgroundColour,
        kShadowColour,
        kGlowColour,
        kPreColour,
        kPostColour,
        kSideColour,
        kGridColour,
        kTagColour,
        kGainColour,
        kSideLoudnessColour,
        kColourNum
    };

    enum SensitivityIdx {
        kMouseWheel,
        kMouseWheelFine,
        kMouseDrag,
        kMouseDragFine,
        kSensitivityNum
    };

    enum SettingIdx {
        kUISettingPanelShow,
        kMatchPanelShow,
        kMatchPanelFit,
        kMatchLowCut,
        kMatchHighCut,
        kMatchShift,
        kMatchFitRunning,
        kSettingNum
    };

    inline std::array kMatchIdentifiers{
        juce::Identifier("ui_setting_panel_show"),
        juce::Identifier("match_panel_show"), juce::Identifier("match_panel_fit"),
        juce::Identifier("match_low_cut"), juce::Identifier("match_high_cut"),
        juce::Identifier("match_shift"), juce::Identifier("match_fit_running")
    };

    enum BoxIdx {
        kOutputBox,
        kAnalyzerBox,
        kDynamicBox,
        kCollisionBox,
        kGeneralBox,
        kBoxNum
    };

    inline std::array kBoxIdentifiers{
        juce::Identifier("output_box"),
        juce::Identifier("analyzer_box"), juce::Identifier("dynamic_box"),
        juce::Identifier("collision_box"), juce::Identifier("general_box")
    };

    static constexpr std::array<std::string, kColourNum> kColourNames{
        "text", "background",
        "shadow", "glow",
        "pre", "post", "side",
        "grid", "tag", "gain", "side_loudness"
    };

    static constexpr size_t kColorMap1Size = 10;
    static constexpr size_t kColorMap2Size = 6;

    inline std::array kColourMaps = {
        std::vector<juce::Colour>{
            juce::Colour(31, 119, 180),
            juce::Colour(255, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 40),
            juce::Colour(148, 103, 189),
            juce::Colour(140, 86, 75),
            juce::Colour(227, 119, 194),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 34),
            juce::Colour(23, 190, 207)
        },
        std::vector<juce::Colour>{
            juce::Colour(224, 136, 75),
            juce::Colour(0, 128, 241),
            juce::Colour(211, 95, 211),
            juce::Colour(41, 216, 215),
            juce::Colour(107, 152, 66),
            juce::Colour(115, 169, 180),
            juce::Colour(28, 136, 61),
            juce::Colour(128, 128, 128),
            juce::Colour(67, 66, 221),
            juce::Colour(232, 65, 48),
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
            juce::Colour(255, 192, 0),
            juce::Colour(252, 18, 197),
            juce::Colour(23, 255, 244),
            juce::Colour(117, 212, 29),
            juce::Colour(0, 59, 255),
            juce::Colour(255, 40, 0),
        }
    };

    static constexpr float kFontTiny = 0.5f;
    static constexpr float kFontSmall = 0.75f;
    static constexpr float kFontNormal = 1.0f;
    static constexpr float kFontLarge = 1.25f;
    static constexpr float kFontHuge = 1.5f;
    static constexpr float kFontHuge2 = 3.0f;
    static constexpr float kFontHuge3 = 4.5f;

    static constexpr int kRefreshFreqHz = 60;

    struct FillRoundedShadowRectangleArgs {
        float blur_radius = 0.5f;
        bool curve_top_left = true, curve_top_right = true, curve_bottom_left = true, curve_bottom_right = true;
        bool fit = true, flip = false;
        bool draw_bright = true, draw_dark = true, draw_main = true;
        juce::Colour main_colour = juce::Colours::white.withAlpha(0.f);
        juce::Colour dark_shadow_color = juce::Colours::white.withAlpha(0.f);
        juce::Colour bright_shadow_color = juce::Colours::white.withAlpha(0.f);
        bool change_main = false, change_dark = false, change_bright = false;
    };

    struct FillShadowEllipseArgs {
        float blur_radius = 0.5f;
        bool fit = true, flip = false;
        bool draw_bright = true, draw_dark = true;
        juce::Colour main_colour = juce::Colours::white.withAlpha(0.f);
        juce::Colour dark_shadow_color = juce::Colours::white.withAlpha(0.f);
        juce::Colour bright_shadow_color = juce::Colours::white.withAlpha(0.f);
        bool change_main = false, change_dark = false, change_bright = false;
    };

    inline std::string formatFloat(const float x, const int precision) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(std::max(0, precision)) << x;
        return stream.str();
    }

    inline std::string fixFormatFloat(const float x, const int length) {
        auto y = std::abs(x);
        if (y < 10) {
            return formatFloat(x, length - 1);
        } else if (y < 100) {
            return formatFloat(x, length - 2);
        } else if (y < 1000) {
            return formatFloat(x, length - 3);
        } else if (y < 10000) {
            return formatFloat(x, length - 4);
        } else {
            return formatFloat(x, length - 5);
        }
    }

    class UIBase {
    public:
        explicit UIBase(juce::AudioProcessorValueTreeState &apvts)
            : state(apvts), font_size_{0.f} {
            loadFromAPVTS();
            updateActualLangIdx();
        }

        void setFontSize(const float fSize) { font_size_ = fSize; }

        inline float getFontSize() const { return font_size_; }

        inline juce::Colour getTextColor() const {
            return custom_colours_[static_cast<size_t>(kTextColour)];
        }

        inline juce::Colour getTextInactiveColor() const { return getTextColor().withAlpha(0.5f); }

        inline juce::Colour getTextHideColor() const { return getTextColor().withAlpha(0.25f); }

        inline juce::Colour getBackgroundColor() const {
            return custom_colours_[static_cast<size_t>(kBackgroundColour)];
        }

        inline juce::Colour getBackgroundInactiveColor() const { return getBackgroundColor().withAlpha(0.8f); }

        inline juce::Colour getBackgroundHideColor() const { return getBackgroundColor().withAlpha(0.5f); }

        inline juce::Colour getDarkShadowColor() const {
            return custom_colours_[static_cast<size_t>(kShadowColour)];
        }

        inline juce::Colour getBrightShadowColor() const {
            return custom_colours_[static_cast<size_t>(kGlowColour)];
        }

        inline juce::Colour getColorMap1(const size_t idx) const {
            return kColourMaps[colour_map1_idx_][idx % kColourMaps[colour_map1_idx_].size()];
        }

        inline juce::Colour getColorMap2(const size_t idx) const {
            return kColourMaps[colour_map2_idx_][idx % kColourMaps[colour_map2_idx_].size()];
        }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> box_bounds,
                                                                    float corner_size,
                                                                    const FillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics &g,
                                                          juce::Rectangle<float> box_bounds,
                                                          float corner_size,
                                                          const FillRoundedShadowRectangleArgs &margs) const;

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                               juce::Rectangle<float> box_bounds,
                                                               float corner_size,
                                                               const FillRoundedShadowRectangleArgs &margs) const;

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                           float corner_size,
                                                           const FillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics &g,
                                                 juce::Rectangle<float> box_bounds,
                                                 float corner_size,
                                                 const FillShadowEllipseArgs &margs) const;

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                                float corner_size,
                                                                const FillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics &g,
                                                      juce::Rectangle<float> box_bounds,
                                                      float corner_size,
                                                      const FillShadowEllipseArgs &margs) const;

        juce::Colour getColourByIdx(ColourIdx idx) const {
            return custom_colours_[static_cast<size_t>(idx)];
        }

        void setColourByIdx(const ColourIdx idx, const juce::Colour colour) {
            custom_colours_[static_cast<size_t>(idx)] = colour;
        }

        inline float getSensitivity(const SensitivityIdx idx) const {
            return wheel_sensitivity_[static_cast<size_t>(idx)];
        }

        void setSensitivity(const float v, const SensitivityIdx idx) {
            wheel_sensitivity_[static_cast<size_t>(idx)] = v;
        }

        size_t getRotaryStyleID() const {
            return rotary_style_id_;
        }

        juce::Slider::SliderStyle getRotaryStyle() const {
            return zlstate::rotaryStyle::styles[rotary_style_id_];
        }

        void setRotaryStyleID(const size_t x) {
            rotary_style_id_ = x;
        }

        float getRotaryDragSensitivity() const {
            return rotary_drag_sensitivity_;
        }

        void setRotaryDragSensitivity(const float x) {
            rotary_drag_sensitivity_ = x;
        }

        int getRefreshRateMS() const {
            return zlstate::refreshRate::ms[refresh_rate_id_.load()];
        }

        size_t getRefreshRateID() const {
            return refresh_rate_id_.load();
        }

        void setRefreshRateID(const size_t x) {
            refresh_rate_id_.store(x);
        }

        float getFFTExtraTilt() const {
            return fft_extra_tilt_.load();
        }

        void setFFTExtraTilt(const float x) {
            fft_extra_tilt_.store(x);
        }

        float getFFTExtraSpeed() const {
            return fft_extra_speed_.load();
        }

        void setFFTExtraSpeed(const float x) {
            fft_extra_speed_.store(x);
        }

        float getSingleCurveThickness() const {
            return single_curve_thickness_.load();
        }

        void setSingleCurveThickness(const float x) {
            single_curve_thickness_.store(x);
        }

        float getSumCurveThickness() const {
            return sum_curve_thickness_.load();
        }

        void setSumCurveThickness(const float x) {
            sum_curve_thickness_.store(x);
        }

        void loadFromAPVTS();

        void saveToAPVTS() const;

        bool getIsBandSelected(const size_t x) const { return is_band_selected_[x].load(); }

        void setIsBandSelected(const size_t x, const bool f) { is_band_selected_[x].store(f); }

        bool getIsMouseWheelShiftReverse() const { return is_mouse_wheel_shift_reverse_.load(); }

        void setIsMouseWheelShiftReverse(const bool x) { is_mouse_wheel_shift_reverse_.store(x); }

        bool getIsSliderDoubleClickOpenEditor() const { return is_slider_double_click_open_editor_.load(); }

        void setIsSliderDoubleClickOpenEditor(const bool x) { is_slider_double_click_open_editor_.store(x); }

        int getDefaultPassFilterSlope() const { return default_pass_filter_slope_.load(); }

        void setDefaultPassFilterSlope(const int x) { default_pass_filter_slope_.store(x); }

        size_t getCMap1Idx() const { return colour_map1_idx_; }

        void setCMap1Idx(const size_t x) { colour_map1_idx_ = x; }

        size_t getCMap2Idx() const { return colour_map2_idx_; }

        void setCMap2Idx(const size_t x) { colour_map2_idx_ = x; }

        int getFFTOrderIdx() const { return fft_order_idx_; }

        void setFFTOrderIdx(const int x) { fft_order_idx_ = x; }

        bool getDynLink() const { return dyn_link_.load(); }

        void setDynLink(const bool x) { dyn_link_.store(x); }

        int getRenderingEngine() const { return rendering_engine_.load(); }

        void setRenderingEngine(const int x) { rendering_engine_.store(x); }

        bool getTooltipON() const { return tooltip_on_; }

        void setTooltipON(const bool x) { tooltip_on_ = x; }

        int getLangIdx() const { return static_cast<int>(lang_idx_); }

        void setLangIdx(const int x) { lang_idx_ = static_cast<multilingual::Languages>(x); }

        juce::ValueTree &getValueTree() { return value_tree_; }

        juce::var getProperty(const SettingIdx idx) const {
            return value_tree_.getProperty(kMatchIdentifiers[static_cast<size_t>(idx)]);
        }

        void setProperty(const SettingIdx idx, const juce::var &v) {
            value_tree_.setProperty(kMatchIdentifiers[idx], v, nullptr);
        }

        static bool isProperty(const SettingIdx idx, const juce::Identifier &property) {
            return property == kMatchIdentifiers[static_cast<size_t>(idx)];
        }

        juce::ValueTree &getBoxTree() { return box_tree_; }

        juce::var getBoxProperty(const BoxIdx idx) const {
            return box_tree_.getProperty(kBoxIdentifiers[static_cast<size_t>(idx)]);
        }

        void setBoxProperty(const BoxIdx idx, const juce::var &v) {
            box_tree_.setProperty(kBoxIdentifiers[idx], v, nullptr);
        }

        static bool isBoxProperty(const BoxIdx idx, const juce::Identifier &property) {
            return property == kBoxIdentifiers[static_cast<size_t>(idx)];
        }

        void openOneBox(const BoxIdx idx) {
            for (const auto &midx: {kGeneralBox, kCollisionBox, kDynamicBox, kAnalyzerBox, kOutputBox}) {
                setBoxProperty(midx, midx == idx);
            }
        }

        void closeAllBox() {
            for (const auto &idx: {kGeneralBox, kCollisionBox, kDynamicBox, kAnalyzerBox, kOutputBox}) {
                box_tree_.setProperty(kBoxIdentifiers[idx], false, nullptr);
            }
        }

        std::string getToolTipText(const zlgui::multilingual::Labels label) const {
            switch (actual_lang_idx_) {
                case multilingual::Languages::kLang_en: {
                    return multilingual::en::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_zh_Hans: {
                    return multilingual::zh_Hans::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_zh_Hant: {
                    return multilingual::zh_Hant::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_it: {
                    return multilingual::it::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_ja: {
                    return multilingual::ja::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_de: {
                    return multilingual::de::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_es: {
                    return multilingual::es::kTexts[static_cast<size_t>(label)];
                }
                case multilingual::Languages::kLang_system:
                case multilingual::Languages::kLangNum:
                default: {
                    return multilingual::en::kTexts[static_cast<size_t>(label)];
                }
            }
        }

        void setIsRenderingHardware(const bool x) { is_rendering_hardware_.store(x); }

        bool getIsRenderingHardware() const { return is_rendering_hardware_.load(); }

        void setIsEditorShowing(const bool x) { is_editor_showing_ = x; }

        bool getIsEditorShowing() const { return is_editor_showing_; }

    private:
        juce::AudioProcessorValueTreeState &state;
        juce::ValueTree value_tree_{"ui_setting"};
        juce::ValueTree box_tree_{"box_setting"};

        float font_size_{0.f};
        std::array<juce::Colour, kColourNum> custom_colours_;
        std::array<float, kSensitivityNum> wheel_sensitivity_{1.f, 0.12f, 1.f, .25f};
        size_t rotary_style_id_{0};
        std::atomic<size_t> refresh_rate_id_{2};
        float rotary_drag_sensitivity_{1.f};
        std::atomic<float> fft_extra_tilt_{0.f}, fft_extra_speed_{1.f};
        std::atomic<float> single_curve_thickness_{1.f}, sum_curve_thickness_{1.f};
        std::array<std::atomic<bool>, zlstate::kBandNUM> is_band_selected_{};
        std::atomic<bool> is_mouse_wheel_shift_reverse_{false};
        std::atomic<bool> is_slider_double_click_open_editor_{false};
        std::atomic<int> default_pass_filter_slope_{1};
        std::atomic<bool> dyn_link_{true};
        std::atomic<int> rendering_engine_{1};
        std::atomic<bool> is_rendering_hardware_{true};
        bool is_editor_showing_{false};

        float loadPara(const std::string &id) const {
            return state.getRawParameterValue(id)->load();
        }

        void savePara(const std::string &id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }

        size_t colour_map1_idx_{zlstate::colourMapIdx::kDefaultDark};
        size_t colour_map2_idx_{zlstate::colourMapIdx::kSeabornBrightDark};

        int fft_order_idx_{1};

        bool tooltip_on_{true};
        multilingual::Languages lang_idx_{multilingual::Languages::kLang_en};
        multilingual::Languages actual_lang_idx_{multilingual::Languages::kLang_en};

        void updateActualLangIdx();
    };
}
