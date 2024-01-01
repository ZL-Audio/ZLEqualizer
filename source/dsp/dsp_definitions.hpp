// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/20/23.
//

#ifndef ZLEQUALIZER_DSP_DEFINITIONS_HPP
#define ZLEQUALIZER_DSP_DEFINITIONS_HPP

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlDSP {
    inline auto static const versionHint = 1;

    inline auto static const bandNUM = 1;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
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
    };

    class fType : public ChoiceParameters<fType> {
    public:
        auto static constexpr ID = "f_type";
        auto static constexpr name = "Type";
        inline auto static const choices = juce::StringArray{"Peak", "Low Shelf", "Low Pass",
                                                             "High Shelf", "High Pass", "Notch",
                                                             "Band Pass", "Band Shelf", "Tilt Shelf"};
        int static constexpr defaultI = 0;
        enum {
            peak, lowShelf, lowPass, highShelf, highPass,
            notch, bandPass, bandShelf, tiltShelf, fTypeNUM
        };
    };

    class slope : public ChoiceParameters<slope> {
    public:
        auto static constexpr ID = "slope";
        auto static constexpr name = "Slope";
        inline auto static const choices = juce::StringArray{"6 dB/oct", "12 dB/oct", "24 dB/oct",
                                                             "36 dB/oct", "48 dB/oct", "72 dB/oct", "96 dB/oct"};
        int static constexpr defaultI = 1;
        static constexpr std::array<size_t, 7> orderArray{1, 2, 4, 6, 8, 12, 16};
    };

    class freq : public FloatParameters<freq> {
    public:
        auto static constexpr ID = "freq";
        auto static constexpr name = "Freq";
        inline auto static const range = juce::NormalisableRange<float>(10, 20000, 1.f, 0.23064293761596813f);
        auto static constexpr defaultV = 1000.f;
    };

    class gain : public FloatParameters<gain> {
    public:
        auto static constexpr ID = "gain";
        auto static constexpr name = "Gain";
        inline auto static const range = juce::NormalisableRange<float>(-30, 30, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class Q : public FloatParameters<Q> {
    public:
        auto static constexpr ID = "Q";
        auto static constexpr name = "Q";
        inline auto static const range = juce::NormalisableRange<float>(.025f, 25, .001f, 0.19213519025943943f);
        auto static constexpr defaultV = 0.707f;
    };

    class lrType : public ChoiceParameters<lrType> {
    public:
        auto static constexpr ID = "lr_type";
        auto static constexpr name = "LRType";
        inline auto static const choices = juce::StringArray{"Stereo", "Left", "Right", "Mid", "Side"};
        int static constexpr defaultI = 0;
    };

    enum lrTypes {
        stereo, left, right, mid, side, lrTypeNUM
    };

    class bypass : public BoolParameters<bypass> {
    public:
        auto static constexpr ID = "bypass";
        auto static constexpr name = "Bypass";
        auto static constexpr defaultV = true;
    };

    class dynamicON : public BoolParameters<dynamicON> {
    public:
        auto static constexpr ID = "dynamic_on";
        auto static constexpr name = "Dynamic ON";
        auto static constexpr defaultV = false;
    };

    class targetGain : public FloatParameters<targetGain> {
    public:
        auto static constexpr ID = "target_gain";
        auto static constexpr name = "Gain";
        inline auto static const range = juce::NormalisableRange<float>(-30, 30, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class targetQ : public FloatParameters<targetQ> {
    public:
        auto static constexpr ID = "target_Q";
        auto static constexpr name = "Q";
        inline auto static const range = juce::NormalisableRange<float>(.025f, 25, .001f, 0.19213519025943943f);
        auto static constexpr defaultV = 0.707f;
    };

    class dynamicBypass : public BoolParameters<dynamicBypass> {
    public:
        auto static constexpr ID = "dynamic_bypass";
        auto static constexpr name = "Dynamic Bypass";
        auto static constexpr defaultV = false;
    };

    class threshold : public FloatParameters<threshold> {
    public:
        auto static constexpr ID = "threshold";
        auto static constexpr name = "Threshold (dB)";
        inline auto static const range =
                juce::NormalisableRange<float>(-80.f, 0.f, .1f);
        auto static constexpr defaultV = -40.f;
    };

    class ratio : public FloatParameters<ratio> {
    public:
        auto static constexpr ID = "ratio";
        auto static constexpr name = "Ratio";
        auto static constexpr defaultV = 3.f;
        inline auto static const range =
                juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 0.2160127f);
    };

    class sideFreq : public FloatParameters<sideFreq> {
    public:
        auto static constexpr ID = "side_freq";
        auto static constexpr name = "Freq";
        inline auto static const range = juce::NormalisableRange<float>(10, 20000, 1.f, 0.23064293761596813f);
        auto static constexpr defaultV = 1000.f;
    };

    class attack : public FloatParameters<attack> {
    public:
        auto static constexpr ID = "attack";
        auto static constexpr name = "Attack (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(1.f, 500.f, 0.1f, 0.30103f);
        auto static constexpr defaultV = 10.f;
    };

    class release : public FloatParameters<release> {
    public:
        auto static constexpr ID = "release";
        auto static constexpr name = "Release (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(1.f, 5000.f, 0.1f, 0.30103f);
        auto static constexpr defaultV = 100.f;
    };

    class sideQ : public FloatParameters<sideQ> {
    public:
        auto static constexpr ID = "side_Q";
        auto static constexpr name = "Q";
        inline auto static const range = juce::NormalisableRange<float>(.025f, 25, .001f, 0.19213519025943943f);
        auto static constexpr defaultV = 0.707f;
    };

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(bypass::get(suffix), fType::get(suffix), slope::get(suffix),
                   freq::get(suffix), gain::get(suffix), Q::get(suffix),
                   lrType::get(suffix), dynamicON::get(suffix, false), dynamicBypass::get(suffix),
                   targetGain::get(suffix), targetQ::get(suffix), threshold::get(suffix), ratio::get(suffix),
                   sideFreq::get(suffix), attack::get(suffix), release::get(suffix), sideQ::get(suffix));
    }

    class sideChain : public BoolParameters<sideChain> {
    public:
        auto static constexpr ID = "side_chain";
        auto static constexpr name = "Side Chain";
        auto static constexpr defaultV = false;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        for (int i = 0; i < bandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            addOneBandParas(layout, suffix);
        }
        return layout;
    }

}

#endif //ZLEQUALIZER_DSP_DEFINITIONS_HPP
