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
        static constexpr auto kID = "eq_max_db";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "6 dB", "12 dB", "30 dB"
        };
        static constexpr std::array kDBs = {6.f, 12.f, 30.f};
        static constexpr int kDefaultI = 1;
    };

    class PFFTMinDB : public ChoiceParameters<PFFTMinDB> {
    public:
        static constexpr auto kID = "fft_min_db";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "-60", "-72", "-96", "-120"
        };
        static constexpr std::array kDBs = {-60.f, -72.f, -96.f, -120.f};
        static constexpr int kDefaultI = 1;

        static float getMinDBFromIndex(const float x) {
            return kDBs[static_cast<size_t>(std::round(x))];
        }
    };

    class PFFTPreON : public BoolParameters<PFFTPreON> {
    public:
        static constexpr auto kID = "fft_pre_on";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PFFTPostON : public BoolParameters<PFFTPostON> {
    public:
        static constexpr auto kID = "fft_post_on";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PFFTSideON : public BoolParameters<PFFTSideON> {
    public:
        static constexpr auto kID = "fft_side_on";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PFFTSpeed : public ChoiceParameters<PFFTSpeed> {
    public:
        static constexpr auto kID = "fft_speed";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Very Fast", "Fast", "Medium", "Slow", "Very Slow"
        };
        static constexpr std::array<float, 5> kSpeeds{0.90f, 0.93f, 0.95f, 0.98f, 0.99f};
        static constexpr int kDefaultI = 2;
    };

    class PFFTTilt : public ChoiceParameters<PFFTTilt> {
    public:
        static constexpr auto kID = "fft_tilt";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "0 dB/oct", "1.5 dB/oct", "3 dB/oct", "4.5 dB/oct", "6 dB/oct"
        };
        static constexpr std::array<float, 5> kSlopes{0.f, 1.5f, 3.f, 4.5f, 6.f};
        static constexpr int kDefaultI = 3;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PEQMaxDB::get(), PFFTMinDB::get(),
                   PFFTPreON::get(), PFFTPostON::get(), PFFTSideON::get(),
                   PFFTSpeed::get(), PFFTTilt::get());
        return layout;
    }

    class PWindowW : public FloatParameters<PWindowW> {
    public:
        static constexpr auto kID = "window_w";
        static constexpr auto kName = "";
        static constexpr auto kMinV = 600.f;
        static constexpr auto kMaxV = 6000.f;
        static constexpr auto kDefaultV = 600.f;
        inline static const auto kRange = juce::NormalisableRange<float>(kMinV, kMaxV, 1.f);
    };

    class PWindowH : public FloatParameters<PWindowH> {
    public:
        static constexpr auto kID = "window_h";
        static constexpr auto kName = "";
        static constexpr auto kMinV = 282.f;
        static constexpr auto kMaxV = 6000.f;
        static constexpr auto kDefaultV = 371.f;
        inline static const auto kRange = juce::NormalisableRange<float>(kMinV, kMaxV, 1.f);
    };

    class PWheelSensitivity : public FloatParameters<PWheelSensitivity> {
    public:
        static constexpr auto kID = "wheel_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PWheelFineSensitivity : public FloatParameters<PWheelFineSensitivity> {
    public:
        static constexpr auto kID = "wheel_fine_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .12f;
    };

    class PWheelShiftReverse : public ChoiceParameters<PWheelShiftReverse> {
    public:
        static constexpr auto kID = "wheel_shift_reverse";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "No Change", "Reverse"
        };
        static constexpr int kDefaultI = 0;
    };

    class PDragSensitivity : public FloatParameters<PDragSensitivity> {
    public:
        static constexpr auto kID = "drag_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PDragFineSensitivity : public FloatParameters<PDragFineSensitivity> {
    public:
        static constexpr auto kID = "drag_fine_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .25f;
    };

    class PRotaryStyle : public ChoiceParameters<PRotaryStyle> {
    public:
        static constexpr auto kID = "rotary_style";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        static constexpr int kDefaultI = 3;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class PRotaryDragSensitivity : public FloatParameters<PRotaryDragSensitivity> {
    public:
        static constexpr auto kID = "rotary_drag_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        static constexpr auto kDefaultV = 10.f;
    };

    class PSliderDoubleClickFunc : public ChoiceParameters<PSliderDoubleClickFunc> {
    public:
        static constexpr auto kID = "slider_double_click_func";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Return Default", "Open Editor"
        };
        static constexpr int kDefaultI = 1;
    };

    class PTargetRefreshSpeed : public ChoiceParameters<PTargetRefreshSpeed> {
    public:
        static constexpr auto kID = "target_refresh_speed_id";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "120 Hz", "90 Hz", "60 Hz", "30 Hz", "15 Hz"
        };
        static constexpr std::array<double, 5> kRates{120.0, 90.0, 60.0, 30.0, 15.0};
        static constexpr int kDefaultI = 3;
    };

    class PFFTExtraTilt : public FloatParameters<PFFTExtraTilt> {
    public:
        static constexpr auto kID = "fft_extra_tilt";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(-4.5f, 4.5f, .01f);
        static constexpr auto kDefaultV = 0.f;
    };

    class PFFTExtraSpeed : public FloatParameters<PFFTExtraSpeed> {
    public:
        static constexpr auto kID = "fft_extra_speed";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 2.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PMagCurveThickness : public FloatParameters<PMagCurveThickness> {
    public:
        static constexpr auto kID = "mag_curve_thickness";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PEQCurveThickness : public FloatParameters<PEQCurveThickness> {
    public:
        static constexpr auto kID = "eq_curve_thickness";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PTooltipLang : public ChoiceParameters<PTooltipLang> {
    public:
        static constexpr auto kID = "tool_tip_lang";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
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
        static constexpr int kDefaultI = 1;
    };

    class PColourMapIdx : public ChoiceParameters<PColourMapIdx> {
    public:
        static constexpr auto kID = "colour_map_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
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

        static constexpr int kDefaultI = 0;
    };

    class PColourMap1Idx : public ChoiceParameters<PColourMap1Idx> {
    public:
        static constexpr auto kID = "colour_map_1_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = PColourMapIdx::kChoices;
        static constexpr int kDefaultI = 1;
    };

    class PColourMap2Idx : public ChoiceParameters<PColourMap2Idx> {
    public:
        static constexpr auto kID = "colour_map_2_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = PColourMapIdx::kChoices;
        static constexpr int kDefaultI = 5;
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
