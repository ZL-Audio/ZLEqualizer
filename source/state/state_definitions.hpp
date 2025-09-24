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

namespace zlstate {
    inline static constexpr int kVersionHint = 1;

#ifdef ZL_EQ_BAND_NUM
    inline static constexpr size_t kBandNum = ZL_EQ_BAND_NUM;
#else
    inline static constexpr size_t kBandNum = 24;
#endif

    // float
    template <class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID, kVersionHint),
                                                               T::kName, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool meta,
                                                              const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                    withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::kRange.convertTo0to1(x);
        }
    };

    // bool
    template <class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID, kVersionHint),
                                                              T::kName, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, const bool meta,
                                                             const bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                   withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template <class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID, kVersionHint),
                                                                T::kName, T::kChoices, T::kDefaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool meta,
                                                               const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                     withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::kChoices.size() - 1);
        }
    };

    class PEQMaxDB : public ChoiceParameters<PEQMaxDB> {
    public:
        auto static constexpr kID = "eq_max_db";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "6 dB", "12 dB", "30 dB"
        };
        static constexpr std::array kDBs = {6.f, 12.f, 30.f};
        int static constexpr kDefaultI = 1;
    };

    class PEQMinDB : public ChoiceParameters<PEQMinDB> {
    public:
        auto static constexpr kID = "eq_min_db";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "-60", "-72", "-96", "-120"
        };
        static constexpr std::array kDBs = {-60.f, -72.f, -96.f, -120.f};
        int static constexpr kDefaultI = 1;

        static float getMinDBFromIndex(const float x) {
            return kDBs[static_cast<size_t>(std::round(x))];
        }
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PEQMaxDB::get(), PEQMinDB::get());
        return layout;
    }

    class PWindowW : public FloatParameters<PWindowW> {
    public:
        auto static constexpr kID = "window_w";
        auto static constexpr kName = "";
        inline static constexpr float minV = 600.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 600.f;
        inline auto static const kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWindowH : public FloatParameters<PWindowH> {
    public:
        auto static constexpr kID = "window_h";
        auto static constexpr kName = "";
        inline static constexpr float minV = 282.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 371.f;
        inline auto static const kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWheelSensitivity : public FloatParameters<PWheelSensitivity> {
    public:
        auto static constexpr kID = "wheel_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PWheelFineSensitivity : public FloatParameters<PWheelFineSensitivity> {
    public:
        auto static constexpr kID = "wheel_fine_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr kDefaultV = .12f;
    };

    class PWheelShiftReverse : public ChoiceParameters<PWheelShiftReverse> {
    public:
        auto static constexpr kID = "wheel_shift_reverse";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "No Change", "Reverse"
        };
        int static constexpr kDefaultI = 0;
    };

    class PDragSensitivity : public FloatParameters<PDragSensitivity> {
    public:
        auto static constexpr kID = "drag_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PDragFineSensitivity : public FloatParameters<PDragFineSensitivity> {
    public:
        auto static constexpr kID = "drag_fine_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr kDefaultV = .25f;
    };

    class PRotaryStyle : public ChoiceParameters<PRotaryStyle> {
    public:
        auto static constexpr kID = "rotary_style";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        int static constexpr kDefaultI = 3;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class PRotaryDragSensitivity : public FloatParameters<PRotaryDragSensitivity> {
    public:
        auto static constexpr kID = "rotary_drag_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        auto static constexpr kDefaultV = 10.f;
    };

    class PSliderDoubleClickFunc : public ChoiceParameters<PSliderDoubleClickFunc> {
    public:
        auto static constexpr kID = "slider_double_click_func";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Return Default", "Open Editor"
        };
        int static constexpr kDefaultI = 1;
    };

    class PTargetRefreshSpeed : public ChoiceParameters<PTargetRefreshSpeed> {
    public:
        auto static constexpr kID = "target_refresh_speed_id";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "120 Hz", "90 Hz", "60 Hz", "30 Hz", "15 Hz"
        };
        static constexpr std::array<double, 5> kRates{120.0, 90.0, 60.0, 30.0, 15.0};
        int static constexpr kDefaultI = 3;
    };

    class PFFTExtraTilt : public FloatParameters<PFFTExtraTilt> {
    public:
        auto static constexpr kID = "fft_extra_tilt";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(-4.5f, 4.5f, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PFFTExtraSpeed : public FloatParameters<PFFTExtraSpeed> {
    public:
        auto static constexpr kID = "fft_extra_speed";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 2.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PMagCurveThickness : public FloatParameters<PMagCurveThickness> {
    public:
        auto static constexpr kID = "mag_curve_thickness";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PEQCurveThickness : public FloatParameters<PEQCurveThickness> {
    public:
        auto static constexpr kID = "eq_curve_thickness";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PTooltipLang : public ChoiceParameters<PTooltipLang> {
    public:
        auto static constexpr kID = "tool_tip_lang";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Off",
            "System",
            "English",
            juce::String(juce::CharPointer_UTF8("简体中文")),
            juce::String(juce::CharPointer_UTF8("繁體中文")),
            juce::String(juce::CharPointer_UTF8("Italiano")),
            juce::String(juce::CharPointer_UTF8("日本語")),
            juce::String(juce::CharPointer_UTF8("Deutsch")),
            juce::String(juce::CharPointer_UTF8("Español"))
        };
        int static constexpr kDefaultI = 1;
    };

    class PColourMapIdx : public ChoiceParameters<PColourMapIdx> {
    public:
        auto static constexpr kID = "colour_map_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Default Light", "Default Dark",
            "Seaborn Normal Light", "Seaborn Normal Dark",
            "Seaborn Bright Light", "Seaborn Bright Dark"
        };

        enum ColourMapName {
            kDefaultLight,
            kDefaultDark,
            kSeabornNormalLight,
            kSeabornNormalDark,
            kSeabornBrightLight,
            kSeabornBrightDark,
            kColourMapNum
        };

        int static constexpr kDefaultI = 0;
    };

    class PColourMap1Idx : public ChoiceParameters<PColourMap1Idx> {
    public:
        auto static constexpr kID = "colour_map_1_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = PColourMapIdx::kChoices;
        int static constexpr kDefaultI = 1;
    };

    class PColourMap2Idx : public ChoiceParameters<PColourMap2Idx> {
    public:
        auto static constexpr kID = "colour_map_2_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = PColourMapIdx::kChoices;
        int static constexpr kDefaultI = 5;
    };

    inline void addOneColour(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                             const std::string& suffix = "",
                             const int red = 0, const int green = 0, const int blue = 0,
                             const bool add_opacity = false, const float opacity = 1.f) {
        layout.add(std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_r", kVersionHint), "",
                       0, 255, red),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_g", kVersionHint), "",
                       0, 255, green),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_b", kVersionHint), "",
                       0, 255, blue));
        if (add_opacity) {
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(suffix + "_o", kVersionHint), "",
                juce::NormalisableRange<float>(0.f, 1.f, .01f), opacity));
        }
    }

    static constexpr std::array<std::string_view, 9> kColourNames{
        "text", "background",
        "shadow", "glow",
        "pre", "post", "reduction",
        "computer", "grid"
    };

    struct ColourDefaultSetting {
        int r, g, b;
        bool has_opacity;
        float opacity;
    };

    static constexpr std::array<ColourDefaultSetting, 9> kColourDefaults{
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, 1.f},
        ColourDefaultSetting{(255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, true, 1.f},
        ColourDefaultSetting{0, 0, 0, true, 1.f},
        ColourDefaultSetting{70, 66, 62, true, 1.f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, .25f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, 1.f},
        ColourDefaultSetting{252, 18, 197, true, 1.f},
        ColourDefaultSetting{255, 165, 0, true, 1.f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, .1f}
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PWindowW::get(), PWindowH::get(),
                   PWheelSensitivity::get(), PWheelFineSensitivity::get(), PWheelShiftReverse::get(),
                   PDragSensitivity::get(), PDragFineSensitivity::get(),
                   PRotaryStyle::get(), PRotaryDragSensitivity::get(),
                   PSliderDoubleClickFunc::get(),
                   PTargetRefreshSpeed::get(),
                   PFFTExtraTilt::get(), PFFTExtraSpeed::get(),
                   PMagCurveThickness::get(), PEQCurveThickness::get(),
                   PTooltipLang::get());

        for (size_t i = 0; i < kColourNames.size(); ++i) {
            const auto& name = kColourNames[i];
            const auto& dv = kColourDefaults[i];
            addOneColour(layout, std::string(name), dv.r, dv.g, dv.b, dv.has_opacity, dv.opacity);
        }

        layout.add(PColourMap1Idx::get(), PColourMap2Idx::get());
        return layout;
    }
}
