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

namespace zlp {
    inline auto static constexpr kVersionHint = 1;

    inline auto static constexpr kBandNUM = 16;

    template<typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType xMin, const FloatType xMax, const FloatType xMid, const FloatType xInterval) {
        const FloatType rng1{std::log(xMid / xMin) * FloatType(2)};
        const FloatType rng2{std::log(xMax / xMid) * FloatType(2)};
        return {
            xMin, xMax,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * xMin : std::exp((v - FloatType(.5)) * rng2) * xMid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < xMid ? std::log(v / xMin) / rng1 : FloatType(.5) + std::log(v / xMid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = xMin + xInterval * std::round((v - xMin) / xInterval);
                return x <= xMin ? xMin : (x >= xMax ? xMax : x);
            }
        };
    }

    inline juce::NormalisableRange<double> logMidRange(
        const double xMin, const double xMax, const double xMid, const double xInterval) {
        return getLogMidRange<double>(xMin, xMax, xMid, xInterval);
    }

    inline juce::NormalisableRange<float> logMidRange(
        const float xMin, const float xMax, const float xMid, const float xInterval) {
        return getLogMidRange<float>(xMin, xMax, xMid, xInterval);
    }

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, kVersionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(bool meta, const std::string &suffix = "",
                                                              bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, kVersionHint),
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
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, kVersionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(bool meta, const std::string &suffix = "",
                                                             bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, kVersionHint),
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
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, kVersionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(bool meta, const std::string &suffix = "",
                                                               bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, kVersionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size() - 1);
        }
    };

    class fType : public ChoiceParameters<fType> {
    public:
        auto static constexpr ID = "f_type";
        auto static constexpr name = "Type";
        inline auto static const choices = juce::StringArray{
            "Peak", "Low Shelf", "Low Pass",
            "High Shelf", "High Pass", "Notch",
            "Band Pass", "Tilt Shelf"
        };
        int static constexpr defaultI = 0;

        enum {
            kPeak,
            kLowShelf,
            kLowPass,
            kHighShelf,
            kHighPass,
            kNotch,
            kBandPass,
            kTiltShelf,
            kFTypeNUM
        };
    };

    class slope : public ChoiceParameters<slope> {
    public:
        auto static constexpr ID = "slope";
        auto static constexpr name = "Slope";
        inline auto static const choices = juce::StringArray{
            "6 dB/oct", "12 dB/oct", "24 dB/oct",
            "36 dB/oct", "48 dB/oct", "72 dB/oct", "96 dB/oct"
        };
        int static constexpr defaultI = 1;
        static constexpr std::array<size_t, 7> orderArray{1, 2, 4, 6, 8, 12, 16};

        static int convertToIdx(const size_t order) {
            switch (order) {
                case 1: return 0;
                case 2: return 1;
                case 4: return 2;
                case 6: return 3;
                case 8: return 4;
                case 12: return 5;
                case 16: return 6;
                default: return 0;
            }
        }
    };

    class freq : public FloatParameters<freq> {
    public:
        auto static constexpr ID = "freq";
        auto static constexpr name = "Freq";
        inline auto static const range = logMidRange(10.f, 20000.f, 1000.f, 0.1f);
        auto static constexpr defaultV = 1000.f;
    };

    class gain : public FloatParameters<gain> {
    public:
        auto static constexpr ID = "gain";
        auto static constexpr name = "Gain";
        inline auto static const range = juce::NormalisableRange<float>(-30, 30, .01f);
        auto static constexpr defaultV = 0.f;
    };

    class Q : public FloatParameters<Q> {
    public:
        auto static constexpr ID = "Q";
        auto static constexpr name = "Q";
        inline auto static const range = logMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr defaultV = 0.707f;
    };

    class lrType : public ChoiceParameters<lrType> {
    public:
        auto static constexpr ID = "lr_type";
        auto static constexpr name = "LRType";
        inline auto static const choices = juce::StringArray{"Stereo", "Left", "Right", "Mid", "Side"};
        int static constexpr defaultI = 0;

        enum lrTypes {
            kStereo,
            kLeft,
            kRight,
            kMid,
            kSide
        };
    };

    class bypass : public BoolParameters<bypass> {
    public:
        auto static constexpr ID = "bypass";
        auto static constexpr name = "Bypass";
        auto static constexpr defaultV = true;
    };

    class solo : public BoolParameters<solo> {
    public:
        auto static constexpr ID = "solo";
        auto static constexpr name = "Solo";
        auto static constexpr defaultV = false;
    };

    class dynamicON : public BoolParameters<dynamicON> {
    public:
        auto static constexpr ID = "dynamic_on";
        auto static constexpr name = "Dynamic ON";
        auto static constexpr defaultV = false;
    };

    class dynamicLearn : public BoolParameters<dynamicLearn> {
    public:
        auto static constexpr ID = "dynamic_learn";
        auto static constexpr name = "Dynamic Learn";
        auto static constexpr defaultV = false;
    };

    class targetGain : public FloatParameters<targetGain> {
    public:
        auto static constexpr ID = "target_gain";
        auto static constexpr name = "Target Gain";
        inline auto static const range = juce::NormalisableRange<float>(-30, 30, .01f);
        auto static constexpr defaultV = 0.f;
    };

    class targetQ : public FloatParameters<targetQ> {
    public:
        auto static constexpr ID = "target_Q";
        auto static constexpr name = "Target Q";
        inline auto static const range = logMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr defaultV = 0.707f;
    };

    class dynamicBypass : public BoolParameters<dynamicBypass> {
    public:
        auto static constexpr ID = "dynamic_bypass";
        auto static constexpr name = "Dynamic Bypass";
        auto static constexpr defaultV = false;
    };

    class dynamicRelative : public BoolParameters<dynamicRelative> {
    public:
        auto static constexpr ID = "dynamic_relative";
        auto static constexpr name = "Dynamic Relative";
        auto static constexpr defaultV = false;
    };

    class sideSwap : public BoolParameters<sideSwap> {
    public:
        auto static constexpr ID = "side_swap";
        auto static constexpr name = "Side Swap";
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

    class kneeW : public FloatParameters<kneeW> {
    public:
        auto static constexpr ID = "knee_width";
        auto static constexpr name = "Knee Width";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 1.f, .01f, .5f);
        auto static constexpr defaultV = 0.25f;

        inline static float formatV(const float x) { return std::max(x * 60, .1f); }

        inline static double formatV(const double x) { return std::max(x * 60, 0.1); }
    };

    class sideFreq : public FloatParameters<sideFreq> {
    public:
        auto static constexpr ID = "side_freq";
        auto static constexpr name = "Side Freq";
        inline auto static const range = logMidRange(10.f, 20000.f, 1000.f, 0.1f);
        auto static constexpr defaultV = 1000.f;
    };

    class attack : public FloatParameters<attack> {
    public:
        auto static constexpr ID = "attack";
        auto static constexpr name = "Attack (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.3010299956639812f);
        auto static constexpr defaultV = 50.f;
    };

    class release : public FloatParameters<release> {
    public:
        auto static constexpr ID = "release";
        auto static constexpr name = "Release (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 5000.f, 0.1f, 0.3010299956639812f);
        auto static constexpr defaultV = 500.f;
    };

    class sideQ : public FloatParameters<sideQ> {
    public:
        auto static constexpr ID = "side_Q";
        auto static constexpr name = "Side Q";
        inline auto static const range = logMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr defaultV = 0.707f;
    };

    class sideSolo : public BoolParameters<sideSolo> {
    public:
        auto static constexpr ID = "side_solo";
        auto static constexpr name = "Side Solo";
        auto static constexpr defaultV = false;
    };

    class dynLookahead : public FloatParameters<dynLookahead> {
    public:
        auto static constexpr ID = "dyn_lookahead";
        auto static constexpr name = "Dynamic Lookahead";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 20.f, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class dynRMS : public FloatParameters<dynRMS> {
    public:
        auto static constexpr ID = "dyn_rms";
        auto static constexpr name = "Dynamic RMS";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 40.f, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class effectON : public ChoiceParameters<effectON> {
    public:
        auto static constexpr ID = "effect_on";
        auto static constexpr name = "Effect ON";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 1;
    };

    class phaseFlip : public ChoiceParameters<phaseFlip> {
    public:
        auto static constexpr ID = "phase_flip";
        auto static constexpr name = "Phase Flip";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class autoGain : public ChoiceParameters<autoGain> {
    public:
        auto static constexpr ID = "auto_gain";
        auto static constexpr name = "Auto Gain";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class staticAutoGain : public ChoiceParameters<staticAutoGain> {
    public:
        auto static constexpr ID = "static_auto_gain";
        auto static constexpr name = "Auto Gain";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class dynSmooth : public FloatParameters<dynSmooth> {
    public:
        auto static constexpr ID = "dyn_smooth";
        auto static constexpr name = "Dynamic Smooth";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, .01f);
        auto static constexpr defaultV = 0.f;
    };

    class scale : public FloatParameters<scale> {
    public:
        auto static constexpr ID = "scale";
        auto static constexpr name = "Scale";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 200.f, .1f);
        auto static constexpr defaultV = 100.f;

        inline static float formatV(const float x) { return x / 100; }

        inline static double formatV(const double x) { return x / 100; }
    };

    class outputGain : public FloatParameters<outputGain> {
    public:
        auto static constexpr ID = "output_gain";
        auto static constexpr name = "Output Gain";
        inline auto static const range =
                juce::NormalisableRange<float>(-16.f, 16.f, .01f, 0.5, true);
        auto static constexpr defaultV = 0.f;
    };

    class singleDynLink : public BoolParameters<singleDynLink> {
    public:
        auto static constexpr ID = "single_dyn_link";
        auto static constexpr name = "Dynamic Link";
        auto static constexpr defaultV = false;
    };

    inline void addOneBandParas(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                                const std::string &suffix = "") {
        layout.add(bypass::get(suffix), solo::get(true, suffix, false),
                   fType::get(suffix), slope::get(suffix),
                   freq::get(true, suffix, true), gain::get(suffix), Q::get(true, suffix, true),
                   lrType::get(suffix),
                   dynamicON::get(true, suffix, false), dynamicLearn::get(true, suffix, false),
                   dynamicBypass::get(suffix), sideSolo::get(true, suffix, false),
                   dynamicRelative::get(suffix, false), sideSwap::get(suffix, false),
                   targetGain::get(suffix), targetQ::get(suffix), threshold::get(suffix), kneeW::get(suffix),
                   sideFreq::get(suffix), attack::get(suffix), release::get(suffix), sideQ::get(suffix),
                   singleDynLink::get(true, suffix, false));
    }

    class sideChain : public BoolParameters<sideChain> {
    public:
        auto static constexpr ID = "side_chain";
        auto static constexpr name = "Side Chain";
        auto static constexpr defaultV = false;
    };

    class filterStructure : public ChoiceParameters<filterStructure> {
    public:
        auto static constexpr ID = "filter_structure";
        auto static constexpr name = "Filter Structure";
        inline auto static const choices = juce::StringArray{
            "Minimum Phase", "State Variable", "Parallel",
            "Matched Phase", "Mixed Phase", "Linear Phase"
        };

        enum FilterStructure {
            kMinimum, kSVF, kParallel, kMatched, kMixed, kLinear
        };

        int static constexpr defaultI = 0;
    };

    class dynHQ : public ChoiceParameters<dynHQ> {
    public:
        auto static constexpr ID = "dyn_hq";
        auto static constexpr name = "Dynamic HQ";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class zeroLatency : public ChoiceParameters<zeroLatency> {
    public:
        auto static constexpr ID = "zero_latency";
        auto static constexpr name = "Zero Latency";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class loudnessMatcherON : public BoolParameters<loudnessMatcherON> {
    public:
        auto static constexpr ID = "loudness_matcher_on";
        auto static constexpr name = "LoudnessMatcherON";
        auto static constexpr defaultV = false;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        for (int i = 0; i < kBandNUM; ++i) {
            auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
            addOneBandParas(layout, suffix);
        }
        layout.add(sideChain::get(),
                   dynLookahead::get(), dynRMS::get(), dynSmooth::get(),
                   effectON::get(), phaseFlip::get(), staticAutoGain::get(), autoGain::get(),
                   scale::get(), outputGain::get(),
                   filterStructure::get(), dynHQ::get(), zeroLatency::get(),
                   loudnessMatcherON::get());
        return layout;
    }

    inline std::string appendSuffix(const std::string &s, const size_t i) {
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        return s + suffix;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter *para, float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}
