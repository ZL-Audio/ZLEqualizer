// Copyright (C) 2023 - zsliu98
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

    class uiStyle : public FloatParameters<uiStyle> {
    public:
        auto static constexpr ID = "ui_style";
        auto static constexpr name = "NA";
        inline static constexpr int minV = 0;
        inline static constexpr int maxV = 1;
        inline static constexpr int defaultV = 1;
        inline auto static const range = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowW : public FloatParameters<windowW> {
    public:
        auto static constexpr ID = "window_w";
        auto static constexpr name = "NA";
        inline static constexpr int minV = 600;
        inline static constexpr int maxV = 6000;
        inline static constexpr int defaultV = 704;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowH : public FloatParameters<windowH> {
    public:
        auto static constexpr ID = "window_h";
        auto static constexpr name = "NA";
        inline static constexpr int minV = 375;
        inline static constexpr int maxV = 3750;
        inline static constexpr int defaultV = 440;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
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
        inline auto static const range = juce::NormalisableRange<float>(0.f, 4.f, .001f, 0.5f);
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

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(uiStyle::get(),
                   windowW::get(), windowH::get());
        return layout;
    }
}

#endif //ZL_STATE_DEFINITIONS_H
