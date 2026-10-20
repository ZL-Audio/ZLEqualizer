// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace zlgui {
    static constexpr float kFontTiny = 0.5f;
    static constexpr float kFontSmall = 0.75f;
    static constexpr float kFontNormal = 1.0f;
    static constexpr float kFontLarge = 1.25f;
    static constexpr float kFontHuge = 1.5f;
    static constexpr float kFontHuge2 = 3.0f;
    static constexpr float kFontHuge3 = 4.5f;

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

    enum SensitivityIdx {
        kMouseWheel,
        kMouseWheelFine,
        kMouseSlider,
        kMouseSliderFine,
        kMouseDragger,
        kMouseDraggerFine,
        kMouseWheelCombobox,
        kSensitivityNum
    };

    enum class ThemeColour {
        kText,
        kBackground,
        kShadow,
        kGlow,
        kCount
    };

    class UIBase {
    public:
        juce::ReferenceCountedObjectPtr<juce::Typeface> font_;

        UIBase(std::vector<std::vector<juce::Colour>> colour_maps, juce::StringArray colour_map_names) :
            colour_maps_(std::move(colour_maps)), colour_map_names_(std::move(colour_map_names)) {
            jassert(colour_maps_.size() == static_cast<size_t>(colour_map_names_.size()));
        }

        juce::Colour getThemeColour(const ThemeColour idx) const {
            return theme_colours_[static_cast<size_t>(idx)];
        }

        void setThemeColour(const ThemeColour idx, const juce::Colour colour) {
            theme_colours_[static_cast<size_t>(idx)] = colour;
        }

        const juce::StringArray& getColourMapNames() const {
            return colour_map_names_;
        }

        const std::vector<juce::Colour>& getColourMap(const size_t idx) const {
            return colour_maps_[idx];
        }

        void setFontMode(const size_t font_mode) {
            font_mode_ = font_mode;
        }

        size_t getFontMode() const {
            return font_mode_;
        }

        void setFontScale(const float font_scale) {
            font_scale_ = font_scale;
        }

        float getFontScale() const {
            return font_scale_;
        }

        void setStaticFontSize(const float static_font_size) {
            static_font_size_ = static_font_size;
        }

        float getStaticFontSize() const {
            return static_font_size_;
        }

        void setFontSize(const float font_size) {
            font_size_ = font_size;
        }

        float getFontSize() const {
            return font_size_;
        }

        juce::Colour getTextColour() const {
            return getThemeColour(ThemeColour::kText);
        }

        juce::Colour getBackgroundColour() const {
            return getThemeColour(ThemeColour::kBackground);
        }

        juce::Colour getColourBlendedWithBackground(const juce::Colour colour, const float alpha) const {
            const auto background_color = getBackgroundColour();
            return juce::Colour::fromFloatRGBA(
                alpha * colour.getFloatRed() + (1.f - alpha) * background_color.getFloatRed(),
                alpha * colour.getFloatGreen() + (1.f - alpha) * background_color.getFloatGreen(),
                alpha * colour.getFloatBlue() + (1.f - alpha) * background_color.getFloatBlue(),
                1.f);
        }

        juce::Colour getDarkShadowColour() const {
            return getThemeColour(ThemeColour::kShadow);
        }

        juce::Colour getBrightShadowColour() const {
            return getThemeColour(ThemeColour::kGlow);
        }

        juce::Colour getColourMap1(const size_t idx) const {
            return colour_maps_[colour_map1_idx_][idx % colour_maps_[colour_map1_idx_].size()];
        }

        juce::Colour getColourMap2(const size_t idx) const {
            return colour_maps_[colour_map2_idx_][idx % colour_maps_[colour_map2_idx_].size()];
        }

        float getSensitivity(const SensitivityIdx idx) const {
            return wheel_sensitivity_[static_cast<size_t>(idx)];
        }

        void setSensitivity(const float v, const SensitivityIdx idx) {
            wheel_sensitivity_[static_cast<size_t>(idx)] = v;
        }

        size_t getRotaryStyleID() const {
            return rotary_style_id_;
        }

        juce::Slider::SliderStyle getRotaryStyle() const {
            return kRotaryStyles[rotary_style_id_];
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

        size_t getComboboxAlignment() const {
            return combobox_alignment_.load(std::memory_order::relaxed);
        }

        void setComboboxAlignment(const size_t x) {
            combobox_alignment_.store(x, std::memory_order::relaxed);
        }

        size_t getTooltipLangID() const {
            return tooltip_lang_id_.load(std::memory_order::relaxed);
        }

        void setTooltipLandID(const size_t x) {
            tooltip_lang_id_.store(x, std::memory_order::relaxed);
        }

        bool getIsMouseWheelShiftReverse() const { return is_mouse_wheel_shift_reverse_.load(); }

        void setIsMouseWheelShiftReverse(const bool x) { is_mouse_wheel_shift_reverse_.store(x); }

        bool getIsSliderDoubleClickOpenEditor() const { return is_slider_double_click_open_editor_.load(); }

        void setIsSliderDoubleClickOpenEditor(const bool x) { is_slider_double_click_open_editor_.store(x); }

        size_t getCMap1Idx() const { return colour_map1_idx_; }

        void setCMap1Idx(const size_t x) { colour_map1_idx_ = x; }

        size_t getCMap2Idx() const { return colour_map2_idx_; }

        void setCMap2Idx(const size_t x) { colour_map2_idx_ = x; }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> box_bounds,
                                                                    float corner_size,
                                                                    const FillRoundedShadowRectangleArgs& margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics& g,
                                                          juce::Rectangle<float> box_bounds,
                                                          float corner_size,
                                                          const FillRoundedShadowRectangleArgs& margs) const;

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics& g,
                                                               juce::Rectangle<float> box_bounds,
                                                               float corner_size,
                                                               const FillRoundedShadowRectangleArgs& margs) const;

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                           float corner_size,
                                                           const FillShadowEllipseArgs& margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics& g,
                                                 juce::Rectangle<float> box_bounds,
                                                 float corner_size,
                                                 const FillShadowEllipseArgs& margs) const;

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                                float corner_size,
                                                                const FillShadowEllipseArgs& margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics& g,
                                                      juce::Rectangle<float> box_bounds,
                                                      float corner_size,
                                                      const FillShadowEllipseArgs& margs) const;

    private:
        float font_size_{0.f};
        size_t font_mode_{0};
        float font_scale_{9.f};
        float static_font_size_{0.f};
        std::array<juce::Colour, static_cast<size_t>(ThemeColour::kCount)> theme_colours_;
        std::array<float, kSensitivityNum> wheel_sensitivity_{1.f, .12f, 1.f, .25f, 1.f, .25f, .5f};
        size_t rotary_style_id_{0};
        float rotary_drag_sensitivity_{1.f};
        std::atomic<size_t> combobox_alignment_{1};
        std::atomic<size_t> tooltip_lang_id_{1};
        std::atomic<bool> is_mouse_wheel_shift_reverse_{false};
        std::atomic<bool> is_slider_double_click_open_editor_{false};
        size_t colour_map1_idx_{1};
        size_t colour_map2_idx_{5};
        const std::vector<std::vector<juce::Colour>> colour_maps_;
        const juce::StringArray colour_map_names_;

        static constexpr std::array kRotaryStyles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };
}
