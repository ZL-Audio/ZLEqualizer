// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_STATE_DEFINITIONS_H
#define ZL_STATE_DEFINITIONS_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <BinaryData.h>

namespace zlState {
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

    class ffTOrder : public ChoiceParameters<ffTOrder> {
    public:
        auto static constexpr ID = "fft_order";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Low", "Medium", "High"
        };
        static constexpr std::array<int, 3> orders{10, 11, 12};
        int static constexpr defaultI = 1;
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

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(active::get(suffix));
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(selectedBandIdx::get(), maximumDB::get(),
                   fftPreON::get(), fftPostON::get(), fftSideON::get(),
                   ffTOrder::get(), ffTSpeed::get(), ffTTilt::get(),
                   conflictON::get(), conflictStrength::get(), conflictScale::get());
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
        int static constexpr defaultI = 0;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class rotaryDragSensitivity : public FloatParameters<rotaryDragSensitivity> {
    public:
        auto static constexpr ID = "rotary_darg_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        auto static constexpr defaultV = 10.f;
    };

    class refreshRate : public ChoiceParameters<refreshRate> {
    public:
        auto static constexpr ID = "refresh_rate";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "25 Hz", "30 Hz", "60 Hz", "90 Hz", "120 Hz"
        };
        int static constexpr defaultI = 2;
        inline static std::array<int, 5> ms{
            39, 33, 16, 11, 8
        };
        inline static std::array<float, 5> rates {
            25, 30, 60, 90, 120
        };
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
                   refreshRate::get(),
                   fftExtraTilt::get(), fftExtraSpeed::get(),
                   singleCurveThickness::get(), sumCurveThickness::get());
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
        return layout;
    }
}

#endif //ZL_STATE_DEFINITIONS_H
