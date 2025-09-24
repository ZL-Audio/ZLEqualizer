// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlp {
    inline static constexpr int kVersionHint = 1;

#ifdef ZL_EQ_BAND_NUM
    inline static constexpr size_t kBandNum = ZL_EQ_BAND_NUM;
#else
    inline static constexpr size_t kBandNum = 24;
#endif

    enum FilterStatus {
        kOff, kBypass, kOn
    };

    enum FilterStereo {
        kStereo, kLeft, kRight, kMid, kSide
    };

    enum FilterStructure {
        kMinimum, kSVF, kParallel, kMatched, kMixed, kZero
    };

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        const FloatType rng1{std::log(x_mid / x_min) * FloatType(2)};
        const FloatType rng2{std::log(x_max / x_mid) * FloatType(2)};
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * x_min : std::exp((v - FloatType(.5)) * rng2) * x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid ? std::log(v / x_min) / rng1 : FloatType(.5) + std::log(v / x_mid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRange<FloatType>(x_min, x_max, x_mid, x_interval);
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min + shift, x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertFrom0to1(v) + shift;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertTo0to1(v - shift);
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.snapToLegalValue(v - shift) + shift;
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getSymmetricLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRangeShift<FloatType>(x_min, x_max, x_mid, x_interval, shift);
        auto result_range = juce::NormalisableRange<FloatType>{
            -(x_max + shift), x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0.5)) {
                    return range.convertFrom0to1(v * FloatType(2) - FloatType(1));
                } else {
                    return -range.convertFrom0to1(FloatType(1) - v * FloatType(2));
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.convertTo0to1(v) * FloatType(0.5) + FloatType(0.5);
                } else {
                    return FloatType(0.5) - range.convertTo0to1(-v) * FloatType(0.5);
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.snapToLegalValue(v);
                } else {
                    return -range.snapToLegalValue(-v);
                }
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLinearMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5)
                           ? FloatType(2) * v * (x_mid - x_min) + x_min
                           : FloatType(2) * (v - FloatType(0.5)) * (x_max - x_mid) + x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid
                           ? FloatType(.5) * (v - x_min) / (x_mid - x_min)
                           : FloatType(.5) + FloatType(.5) * (v - x_mid) / (x_max - x_mid);
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

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

    class PFilterStructure : public ChoiceParameters<PFilterStructure> {
    public:
        auto static constexpr kID = "filter_structure";
        auto static constexpr kName = "Filter Structure";
        inline auto static const kChoices = juce::StringArray{
            "Minimum Phase", "State Variable", "Parallel",
            "Matched Phase", "Mixed Phase", "Zero Phase"
        };
        int static constexpr kDefaultI = 0;
    };

    class PExtSide : public BoolParameters<PExtSide> {
    public:
        auto static constexpr kID = "external_side";
        auto static constexpr kName = "External Side";
        auto static constexpr kDefaultV = false;
    };

    class PBypass : public ChoiceParameters<PBypass> {
    public:
        auto static constexpr kID = "bypass";
        auto static constexpr kName = "Bypass";
        inline auto static const kChoices = juce::StringArray{
            "ON", "Bypass"
        };
        int static constexpr kDefaultI = 0;
    };

    class PFilterStatus : public ChoiceParameters<PFilterStatus> {
    public:
        auto static constexpr kID = "filter_status";
        auto static constexpr kName = "Filter Status";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "Bypass", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class PFilterType : public ChoiceParameters<PFilterType> {
    public:
        auto static constexpr kID = "filter_type";
        auto static constexpr kName = "Filter Type";
        inline auto static const kChoices = juce::StringArray{
            "Peak", "Low Shelf", "Low Pass",
            "High Shelf", "High Pass", "Notch",
            "Band Pass", "Tilt Shelf"
        };
        int static constexpr kDefaultI = 0;
    };

    class POrder : public ChoiceParameters<POrder> {
    public:
        auto static constexpr kID = "order";
        auto static constexpr kName = "Order";
        inline auto static const kChoices = juce::StringArray{
            "6 dB/oct", "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct", "72 dB/oct", "96 dB/oct"
        };
        int static constexpr kDefaultI = 1;
        static constexpr std::array<size_t, 8> kOrderArray{1, 2, 4, 6, 8, 12, 16};

        static size_t convertToIdx(const size_t order) {
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

    class PLRMode : public ChoiceParameters<PLRMode> {
    public:
        auto static constexpr kID = "lr_mode";
        auto static constexpr kName = "LRMode";
        inline auto static const kChoices = juce::StringArray{"Stereo", "Left", "Right", "Mid", "Side"};
        int static constexpr kDefaultI = 0;
    };

    class PFreq : public FloatParameters<PFreq> {
    public:
        auto static constexpr kID = "freq";
        auto static constexpr kName = "Freq";
        inline auto static const kRange = getLogMidRange(10.f, 20000.f, 1000.f, 0.1f);
        auto static constexpr kDefaultV = 1000.f;
    };

    class PGain : public FloatParameters<PGain> {
    public:
        auto static constexpr kID = "gain";
        auto static constexpr kName = "Gain";
        inline auto static const kRange = juce::NormalisableRange<float>(-30, 30, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PTargetGain : public FloatParameters<PTargetGain> {
    public:
        auto static constexpr kID = "target_gain";
        auto static constexpr kName = "Target Gain";
        inline auto static const kRange = juce::NormalisableRange<float>(-30, 30, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PQ : public FloatParameters<PQ> {
    public:
        auto static constexpr kID = "q";
        auto static constexpr kName = "Q";
        inline auto static const kRange = getLogMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr kDefaultV = 0.707f;
    };

    class PDynamicON : public BoolParameters<PDynamicON> {
    public:
        auto static constexpr kID = "dynamic_on";
        auto static constexpr kName = "Dynamic ON";
        auto static constexpr kDefaultV = false;
    };

    class PDynamicLearn : public BoolParameters<PDynamicLearn> {
    public:
        auto static constexpr kID = "dynamic_learn";
        auto static constexpr kName = "Dynamic Learn";
        auto static constexpr kDefaultV = false;
    };

    class PDynamicBypass : public BoolParameters<PDynamicBypass> {
    public:
        auto static constexpr kID = "dynamic_bypass";
        auto static constexpr kName = "Dynamic Bypass";
        auto static constexpr kDefaultV = false;
    };

    class PDynamicRelative : public BoolParameters<PDynamicRelative> {
    public:
        auto static constexpr kID = "dynamic_relative";
        auto static constexpr kName = "Dynamic Relative";
        auto static constexpr kDefaultV = false;
    };

    class PSideSwap : public BoolParameters<PSideSwap> {
    public:
        auto static constexpr kID = "side_swap";
        auto static constexpr kName = "Side Swap";
        auto static constexpr kDefaultV = false;
    };

    class PThreshold : public FloatParameters<PThreshold> {
    public:
        auto static constexpr kID = "threshold";
        auto static constexpr kName = "Threshold (dB)";
        inline auto static const kRange = juce::NormalisableRange<float>(-80.f, 0.f, 0.1f);
        auto static constexpr kDefaultV = -40.f;
    };

    class PKneeW : public FloatParameters<PKneeW> {
    public:
        auto static constexpr kID = "knee_width";
        auto static constexpr kName = "Knee Width";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 32.f, .01f, .5f);
        auto static constexpr kDefaultV = 8.f;
    };

    class PAttack : public FloatParameters<PAttack> {
    public:
        auto static constexpr kID = "attack";
        auto static constexpr kName = "Attack";
        inline auto static const kRange = getLogMidRangeShift(20.f, 1020.f, 120.f, 0.01f, -20.f);
        auto static constexpr kDefaultV = 100.f;
    };

    class PRelease : public FloatParameters<PRelease> {
    public:
        auto static constexpr kID = "release";
        auto static constexpr kName = "Release";
        inline auto static const kRange = getLogMidRangeShift(100.f, 5100.f, 600.f, 0.01f, -100.f);
        auto static constexpr kDefaultV = 500.f;
    };

    class PSideFreq : public FloatParameters<PSideFreq> {
    public:
        auto static constexpr kID = "side_freq";
        auto static constexpr kName = "Side Freq";
        inline auto static const kRange = getLogMidRange(10.f, 20000.f, 1000.f, 0.1f);
        auto static constexpr kDefaultV = 1000.f;
    };

    class PSideQ : public FloatParameters<PSideQ> {
    public:
        auto static constexpr kID = "side_q";
        auto static constexpr kName = "Side Q";
        inline auto static const kRange = getLogMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr kDefaultV = 0.707f;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PFilterStructure::get(), PExtSide::get(), PBypass::get());
        for (size_t i = 0; i < kBandNum; ++i) {
            const auto suffix = std::to_string(i);
            layout.add(PFilterStatus::get(suffix), PFilterType::get(suffix), POrder::get(suffix), PLRMode::get(suffix),
                       PFreq::get(suffix), PGain::get(suffix), PTargetGain::get(suffix), PQ::get(suffix),
                       PDynamicON::get(suffix), PDynamicLearn::get(suffix),
                       PDynamicBypass::get(suffix), PDynamicRelative::get(suffix), PSideSwap::get(suffix),
                       PThreshold::get(suffix), PKneeW::get(suffix), PAttack::get(suffix), PRelease::get(suffix),
                       PSideFreq::get(suffix), PSideQ::get(suffix));
        }
        return layout;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter* para, const float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}
