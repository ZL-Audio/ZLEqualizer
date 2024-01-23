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

    inline auto static constexpr bandNUM = 9;

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

    class uiStyle : public FloatParameters<uiStyle> {public:
        auto static constexpr ID = "ui_style";
        auto static constexpr name = "NA";
        inline static const int minV = 0;
        inline static const int maxV = 1;
        inline static const int defaultV = 1;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowW : public FloatParameters<windowW> {
    public:
        auto static constexpr ID = "window_w";
        auto static constexpr name = "NA";
        inline static constexpr int minV = 476;
        inline static constexpr int maxV = 1428;
        inline static constexpr int defaultV = 476;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowH : public FloatParameters<windowH> {
    public:
        auto static constexpr ID = "window_h";
        auto static constexpr name = "NA";
        inline static constexpr int minV = 443;
        inline static constexpr int maxV = 1329;
        inline static constexpr int defaultV = 443;
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
            "1", "2", "3", "4", "5", "6", "7", "8", "9"
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

    class active : public BoolParameters<active> {
    public:
        auto static constexpr ID = "active";
        auto static constexpr name = "Active";
        auto static constexpr defaultV = false;
    };

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(active::get(suffix));
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(selectedBandIdx::get(), maximumDB::get());
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            addOneBandParas(layout, suffix);
        }
        return layout;
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(uiStyle::get(),
                   windowW::get(), windowH::get());
        return layout;
    }

    inline std::string appendSuffix(std::string s, size_t i) {
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        return s + suffix;
    }
}

#endif //ZL_STATE_DEFINITIONS_H
