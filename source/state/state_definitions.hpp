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
    // int
    inline static constexpr auto versionHint = 1;

    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID, versionHint), T::name,
                                                             T::range, T::defaultV, attributes);
        }

        inline static float convertTo01(float x) {
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
        inline static const int minV = 476;
        inline static const int maxV = 1428;
        inline static const int defaultV = 476;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowH : public FloatParameters<windowH> {
    public:
        auto static constexpr ID = "window_h";
        auto static constexpr name = "NA";
        inline static const int minV = 443;
        inline static const int maxV = 1329;
        inline static const int defaultV = 443;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID, versionHint), T::name,
                                                              T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(juce::String label, bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(label);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID, versionHint), T::name,
                                                              T::defaultV, attributes);
        }
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(
                    juce::ParameterID(T::ID, versionHint), T::name, T::choices, T::defaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(juce::String label, bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(label);
            return std::make_unique<juce::AudioParameterChoice>(
                    juce::ParameterID(T::ID, versionHint), T::name, T::choices, T::defaultI, attributes);
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

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(selectedBandIdx::get());
        return layout;
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(uiStyle::get(false),
                   windowW::get(false), windowH::get(false));
        return layout;
    }
}

#endif //ZL_STATE_DEFINITIONS_H
