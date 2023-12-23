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

#ifndef ZLEQUALIZER_DSP_DEFINITIONS_H
#define ZLEQUALIZER_DSP_DEFINITIONS_H

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlDSP {
    inline auto static const versionHint = 1;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name, T::range, T::defaultV, attributes);
        }
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name, T::defaultV, attributes);
        }
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name, T::choices, T::defaultI, attributes);
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

    class fSlope : public ChoiceParameters<fSlope> {
    public:
        auto static constexpr ID = "f_slope";
        auto static constexpr name = "Slope";
        inline auto static const choices = juce::StringArray{"6 dB/oct", "12 dB/oct", "24 dB/oct",
                                                             "36 dB/oct", "48 dB/oct", "60 dB/oct", "72 dB/oct"};
        int static constexpr defaultI = 1;
        enum {
            db06, db12, db24, db36, db48, db60, db72, dbNUM
        };
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

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(fType::get(suffix), fSlope::get(suffix),
                   freq::get(suffix), gain::get(suffix),
                   Q::get(suffix));
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        addOneBandParas(layout, "00");
        return layout;
    }

}

#endif //ZLEQUALIZER_DSP_DEFINITIONS_H
