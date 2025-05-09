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
#include <juce_dsp/juce_dsp.h>
#include <BinaryData.h>

namespace zlstate {
    inline auto static constexpr versionHint = 1;

    inline auto static constexpr bandNUM = 16;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::range.convertTo0to1(x);
        }
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size() - 1);
        }
    };

    class selectedBandIdx : public ChoiceParameters<selectedBandIdx> {
    public:
        auto static constexpr ID = "selected_band_idx";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"
        };
        int static constexpr defaultI = 0;
    };

    class maximumDB : public ChoiceParameters<maximumDB> {
    public:
        auto static constexpr ID = "maximum_db";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "6 dB", "12 dB", "30 dB"
        };
        static constexpr std::array<float, 3> dBs = {6.f, 12.f, 30.f};
        int static constexpr defaultI = 1;
    };

    class minimumFFTDB : public ChoiceParameters<minimumFFTDB> {
    public:
        auto static constexpr ID = "minimum_fft_db";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "-60", "-72", "-96", "-120"
        };
        static constexpr std::array<float, 4> dBs = {-60.f, -72.f, -96.f, -120.f};
        int static constexpr defaultI = 1;
    };

    class fftPreON : public ChoiceParameters<fftPreON> {
    public:
        auto static constexpr ID = "fft_pre_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON", "FRZ"
        };
        int static constexpr defaultI = 1;
    };

    class fftPostON : public ChoiceParameters<fftPostON> {
    public:
        auto static constexpr ID = "fft_post_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON", "FRZ"
        };
        int static constexpr defaultI = 1;
    };

    class fftSideON : public ChoiceParameters<fftSideON> {
    public:
        auto static constexpr ID = "fft_side_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON", "FRZ"
        };
        int static constexpr defaultI = 0;
    };

    class ffTSpeed : public ChoiceParameters<ffTSpeed> {
    public:
        auto static constexpr ID = "fft_speed";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Very Fast", "Fast", "Medium", "Slow", "Very Slow", "Frozen"
        };
        static constexpr std::array<float, 6> speeds{0.90f, 0.93f, 0.95f, 0.98f, 0.99f, 1.0f};
        int static constexpr defaultI = 2;
    };

    class ffTTilt : public ChoiceParameters<ffTTilt> {
    public:
        auto static constexpr ID = "fft_tilt";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "0 dB/oct", "1.5 dB/oct", "3 dB/oct", "4.5 dB/oct", "6 dB/oct"
        };
        static constexpr std::array<float, 5> slopes{0.f, 1.5f, 3.f, 4.5f, 6.f};
        int static constexpr defaultI = 3;
    };

    class active : public BoolParameters<active> {
    public:
        auto static constexpr ID = "active";
        auto static constexpr name = "Active";
        auto static constexpr defaultV = false;
    };

    class conflictON : public ChoiceParameters<conflictON> {
    public:
        auto static constexpr ID = "conflict_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class conflictStrength : public FloatParameters<conflictStrength> {
    public:
        auto static constexpr ID = "conflict_strength";
        auto static constexpr name = "NA";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, .001f);
        auto static constexpr defaultV = 0.5f;

        inline static float formatV(const float x) { return 0.75f * x; }

        inline static double formatV(const double x) { return 0.75 * x; }
    };

    class conflictScale : public FloatParameters<conflictScale> {
    public:
        auto static constexpr ID = "conflict_scale";
        auto static constexpr name = "NA";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 6.f, .001f, 0.3868528072345416f);
        auto static constexpr defaultV = 1.f;
    };

    class matchPanelShow : public ChoiceParameters<matchPanelShow> {
    public:
        auto static constexpr ID = "match_panel_show";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class matchAnalyzerON : public ChoiceParameters<matchAnalyzerON> {
    public:
        auto static constexpr ID = "match_analyzer_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class matchAnalyzerLWeight : public FloatParameters<matchAnalyzerLWeight> {
    public:
        auto static constexpr ID = "match_analyzer_l_weight";
        auto static constexpr name = "NA";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, .01f);
        auto static constexpr defaultV = .5f;
    };

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(active::get(suffix));
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(selectedBandIdx::get(), maximumDB::get(), minimumFFTDB::get(),
                   fftPreON::get(), fftPostON::get(), fftSideON::get(),
                   ffTSpeed::get(), ffTTilt::get(),
                   conflictON::get(), conflictStrength::get(), conflictScale::get(),
                   matchPanelShow::get(),
                   matchAnalyzerON::get(), matchAnalyzerLWeight::get());
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            addOneBandParas(layout, suffix);
        }
        return layout;
    }

    inline std::string appendSuffix(const std::string &s, const size_t i) {
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        return s + suffix;
    }

    class windowW : public FloatParameters<windowW> {
    public:
        auto static constexpr ID = "window_w";
        auto static constexpr name = "NA";
        inline static constexpr float minV = 600.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float defaultV = 704.f;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowH : public FloatParameters<windowH> {
    public:
        auto static constexpr ID = "window_h";
        auto static constexpr name = "NA";
        inline static constexpr float minV = 282.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float defaultV = 440.f;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class wheelSensitivity : public FloatParameters<wheelSensitivity> {
    public:
        auto static constexpr ID = "wheel_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr defaultV = 1.f;
    };

    class wheelFineSensitivity : public FloatParameters<wheelFineSensitivity> {
    public:
        auto static constexpr ID = "wheel_fine_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr defaultV = .12f;
    };

    class wheelShiftReverse : public ChoiceParameters<wheelShiftReverse> {
    public:
        auto static constexpr ID = "wheel_shift_reverse";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "No Change", "Reverse"
        };
        int static constexpr defaultI = 0;
    };

    class dragSensitivity : public FloatParameters<dragSensitivity> {
    public:
        auto static constexpr ID = "drag_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr defaultV = 1.f;
    };

    class dragFineSensitivity : public FloatParameters<dragFineSensitivity> {
    public:
        auto static constexpr ID = "drag_fine_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr defaultV = .25f;
    };

    class rotaryStyle : public ChoiceParameters<rotaryStyle> {
    public:
        auto static constexpr ID = "rotary_style";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        int static constexpr defaultI = 3;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class rotaryDragSensitivity : public FloatParameters<rotaryDragSensitivity> {
    public:
        auto static constexpr ID = "rotary_drag_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        auto static constexpr defaultV = 10.f;
    };

    class sliderDoubleClickFunc : public ChoiceParameters<sliderDoubleClickFunc> {
    public:
        auto static constexpr ID = "slider_double_click_func";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Return Default", "Open Editor"
        };
        int static constexpr defaultI = 1;
    };

    class refreshRate : public ChoiceParameters<refreshRate> {
    public:
        auto static constexpr ID = "refresh_rate";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "25 Hz", "30 Hz", "60 Hz", "90 Hz", "120 Hz"
        };
#if defined(JUCE_MAC)
        int static constexpr defaultI = 2;
#else
        int static constexpr defaultI = 1;
#endif
        inline static std::array<int, 5> ms{
            39, 33, 16, 11, 8
        };
        inline static std::array<float, 5> rates{
            25, 30, 60, 90, 120
        };
    };

    class ffTOrder : public ChoiceParameters<ffTOrder> {
    public:
        auto static constexpr ID = "fft_order";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Low", "Medium", "High"
        };
        static constexpr std::array<size_t, 3> orders{11, 12, 13};
        int static constexpr defaultI = 1;
    };

    class fftExtraTilt : public FloatParameters<fftExtraTilt> {
    public:
        auto static constexpr ID = "fft_extra_tilt";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(-4.5f, 4.5f, .01f);
        inline auto static const doubleRange = juce::NormalisableRange<double>(-4.5, 4.5, .01);
        auto static constexpr defaultV = 0.f;
    };

    class fftExtraSpeed : public FloatParameters<fftExtraSpeed> {
    public:
        auto static constexpr ID = "fft_extra_speed";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 2.f, .01f);
        inline auto static const doubleRange = juce::NormalisableRange<double>(0.0, 2.0, 0.01);
        auto static constexpr defaultV = 1.f;
    };

    class singleCurveThickness : public FloatParameters<singleCurveThickness> {
    public:
        auto static constexpr ID = "single_curve_thickness";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        inline auto static const doubleRange = juce::NormalisableRange<double>(0.0, 4.0, 0.01);
        auto static constexpr defaultV = 1.f;
    };

    class sumCurveThickness : public FloatParameters<sumCurveThickness> {
    public:
        auto static constexpr ID = "sum_curve_thickness";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        inline auto static const doubleRange = juce::NormalisableRange<double>(0.0, 4.0, 0.01);
        auto static constexpr defaultV = 1.f;
    };

    class defaultPassFilterSlope : public ChoiceParameters<defaultPassFilterSlope> {
    public:
        auto static constexpr ID = "default_pass_filter_slope";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "6 dB/oct", "12 dB/oct", "24 dB/oct",
            "36 dB/oct", "48 dB/oct", "72 dB/oct", "96 dB/oct"
        };
        int static constexpr defaultI = 1;
    };

    class dynLink : public ChoiceParameters<dynLink> {
    public:
        auto static constexpr ID = "global_dyn_link";
        auto static constexpr name = "Global Dynamic Link";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 1;
    };

    class renderingEngine : public ChoiceParameters<renderingEngine> {
    public:
        auto static constexpr ID = "renderer";
        auto static constexpr name = "Renderer";
        inline auto static const choices = juce::StringArray{
            "Auto", "Software", "Hardware", "Unknown", "Unknown", "Unknown"
        };
        int static constexpr defaultI = 0;
    };

    class colourMapIdx : public ChoiceParameters<colourMapIdx> {
    public:
        auto static constexpr ID = "colour_map_idx";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Default Light", "Default Dark",
            "Seaborn Normal Light", "Seaborn Normal Dark",
            "Seaborn Bright Light", "Seaborn Bright Dark"
        };

        enum colourMapName {
            defaultLight,
            defaultDark,
            seabornNormalLight,
            seabornNormalDark,
            seabornBrightLight,
            seabornBrightDark,
            colourMapNum
        };

        int static constexpr defaultI = 0;
    };

    class colourMap1Idx : public ChoiceParameters<colourMap1Idx> {
    public:
        auto static constexpr ID = "colour_map_1_idx";
        auto static constexpr name = "";
        inline auto static const choices = colourMapIdx::choices;
        int static constexpr defaultI = 1;
    };

    class colourMap2Idx : public ChoiceParameters<colourMap2Idx> {
    public:
        auto static constexpr ID = "colour_map_2_idx";
        auto static constexpr name = "";
        inline auto static const choices = colourMapIdx::choices;
        int static constexpr defaultI = 5;
    };

    class tooltipON : public ChoiceParameters<tooltipON> {
    public:
        auto static constexpr ID = "tool_tip_on";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{"OFF", "ON"};
        int static constexpr defaultI = 1;
    };

    class tooltipLang : public ChoiceParameters<tooltipLang> {
    public:
        auto static constexpr ID = "tool_tip_lang";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "System",
            "English",
            juce::String(juce::CharPointer_UTF8("简体中文")),
            juce::String(juce::CharPointer_UTF8("繁體中文")),
            juce::String(juce::CharPointer_UTF8("Italiano")),
            juce::String(juce::CharPointer_UTF8("日本語")),
            juce::String(juce::CharPointer_UTF8("Deutsch")),
            juce::String(juce::CharPointer_UTF8("Español"))
        };
        int static constexpr defaultI = 0;
    };

    inline void addOneColour(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                             const std::string &suffix = "",
                             const int red = 0, const int green = 0, const int blue = 0,
                             const bool addOpacity = false, const float opacity = 1.f) {
        layout.add(std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_r", versionHint), "",
                       0, 255, red),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_g", versionHint), "",
                       0, 255, green),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_b", versionHint), "",
                       0, 255, blue));
        if (addOpacity) {
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(suffix + "_o", versionHint), "",
                juce::NormalisableRange<float>(0.f, 1.f, .01f), opacity));
        }
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(windowW::get(), windowH::get(),
                   wheelSensitivity::get(), wheelFineSensitivity::get(), wheelShiftReverse::get(),
                   dragSensitivity::get(), dragFineSensitivity::get(),
                   rotaryStyle::get(), rotaryDragSensitivity::get(),
                   sliderDoubleClickFunc::get(),
                   refreshRate::get(),
                   ffTOrder::get(), fftExtraTilt::get(), fftExtraSpeed::get(),
                   singleCurveThickness::get(), sumCurveThickness::get(),
                   defaultPassFilterSlope::get(), dynLink::get(), renderingEngine::get());
        addOneColour(layout, "pre", 255 - 8, 255 - 9, 255 - 11, true, 0.1f);
        addOneColour(layout, "post", 255 - 8, 255 - 9, 255 - 11, true, 0.1f);
        addOneColour(layout, "side", 252, 18, 197, true, 0.1f);
        addOneColour(layout, "grid", 255 - 8, 255 - 9, 255 - 11, true, .25f);
        addOneColour(layout, "tag", 137, 125, 109, true, 1.f);
        addOneColour(layout, "text", 255 - 8, 255 - 9, 255 - 11, true, 1.f);
        addOneColour(layout, "background", (255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, true, 1.f);
        addOneColour(layout, "shadow", 0, 0, 0, true, 1.f);
        addOneColour(layout, "glow", 70, 66, 62, true, 1.f);
        addOneColour(layout, "gain", 255 - 8, 255 - 9, 255 - 11, true, 1.f);
        addOneColour(layout, "side_loudness", 255 - 8, 255 - 9, 255 - 11, true, .33f);
        layout.add(colourMap1Idx::get(), colourMap2Idx::get());
        layout.add(tooltipON::get(), tooltipLang::get());
        return layout;
    }
}
