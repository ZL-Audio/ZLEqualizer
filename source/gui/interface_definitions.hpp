// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_DEFINES_H
#define ZL_INTERFACE_DEFINES_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../state/state_definitions.hpp"

namespace zlInterface {
    enum colourIdx {
        preColour,
        postColour,
        sideColour,
        gridColour,
        colourNum
    };

    inline std::array<std::string, colourNum> colourNames {"pre", "post", "side", "grid"};

    static constexpr size_t ColorMap1Size = 10;
    static constexpr size_t ColorMap2Size = 6;

    struct UIColors {
        juce::Colour TextColor = juce::Colour(87, 96, 110);
        juce::Colour BackgroundColor = juce::Colour(214, 223, 236);
        juce::Colour TextLightColor = juce::Colour(191, 200, 216);
        juce::Colour DarkShadowColor = juce::Colour(168, 172, 178);
        juce::Colour BrightShadowColor = juce::Colour(237, 246, 255);
        juce::Colour ExtraColor1 = juce::Colour(139, 0, 0);
        std::array<juce::Colour, ColorMap1Size> ColorMap1 = {
            juce::Colour(31, 119, 180),
            juce::Colour(255, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 40),
            juce::Colour(148, 103, 189),
            juce::Colour(140, 86, 75),
            juce::Colour(227, 119, 194),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 34),
            juce::Colour(23, 190, 207)
        };
        std::array<juce::Colour, ColorMap2Size> ColorMap2 = {
            juce::Colour(76, 114, 176),
            juce::Colour(85, 168, 104),
            juce::Colour(196, 78, 82),
            juce::Colour(129, 114, 178),
            juce::Colour(255, 196, 0),
            juce::Colour(100, 181, 205),
        };
    };

    inline const std::array<UIColors, 2> styleColors{
        {
            {},
            {
                .TextColor = juce::Colour(255 - 8, 255 - 9, 255 - 11),
                .BackgroundColor = juce::Colour((255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2),
                .TextLightColor = juce::Colour(137, 125, 109),
                .DarkShadowColor = juce::Colour(0, 0, 0),
                .BrightShadowColor = juce::Colour(255 - 168, 255 - 172, 255 - 178).withMultipliedBrightness(.8f),
                .ExtraColor1 = juce::Colour(255 - 139, 255, 255),
                .ColorMap1 = {
                    juce::Colour(224, 136, 75),
                    juce::Colour(0, 128, 241),
                    juce::Colour(211, 95, 211),
                    juce::Colour(41, 216, 215),
                    juce::Colour(107, 152, 66),
                    juce::Colour(115, 169, 180),
                    juce::Colour(28, 136, 61),
                    juce::Colour(128, 128, 128),
                    juce::Colour(67, 66, 221),
                    juce::Colour(232, 65, 48),
                },
                .ColorMap2 = {
                    juce::Colour(255, 192, 0),
                    juce::Colour(252, 18, 197),
                    juce::Colour(23, 255, 244),
                    juce::Colour(117, 212, 29),
                    juce::Colour(0, 59, 255),
                    juce::Colour(255, 40, 0),
                }
            }
        }
    };

    auto inline const TextColor = juce::Colour(87, 96, 110);
    auto inline const TextInactiveColor = TextColor.withAlpha(0.5f);
    auto inline const TextHideColor = TextColor.withAlpha(0.25f);

    auto inline const BackgroundColor = juce::Colour(214, 223, 236);
    auto inline const BackgroundInactiveColor = BackgroundColor.withAlpha(0.8f);
    auto inline const BackgroundHideColor = BackgroundColor.withAlpha(0.5f);
    auto inline const BackgroundInvisibleColor = BackgroundColor.withAlpha(0.25f);

    auto inline const DarkShadowColor = juce::Colour(168, 172, 178);
    auto inline const BrightShadowColor = juce::Colour(237, 246, 255);

    auto inline constexpr FontTiny = 0.5f;
    auto inline constexpr FontSmall = 0.75f;
    auto inline constexpr FontNormal = 1.0f;
    auto inline constexpr FontLarge = 1.25f;
    auto inline constexpr FontHuge = 1.5f;
    auto inline constexpr FontHuge2 = 3.0f;
    auto inline constexpr FontHuge3 = 4.5f;

    auto inline constexpr RefreshFreqHz = 60;

    struct fillRoundedShadowRectangleArgs {
        float blurRadius = 0.5f;
        bool curveTopLeft = true, curveTopRight = true, curveBottomLeft = true, curveBottomRight = true;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true, drawMain = true;
        juce::Colour mainColour = BackgroundColor;
        juce::Colour darkShadowColor = DarkShadowColor;
        juce::Colour brightShadowColor = BrightShadowColor;
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    struct fillShadowEllipseArgs {
        float blurRadius = 0.5f;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true;
        juce::Colour mainColour = BackgroundColor;
        juce::Colour darkShadowColor = DarkShadowColor;
        juce::Colour brightShadowColor = BrightShadowColor;
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    inline std::string formatFloat(float x, int precision) {
        std::stringstream stream;
        precision = std::max(0, precision);
        stream << std::fixed << std::setprecision(precision) << x;
        return stream.str();
    }

    inline std::string fixFormatFloat(float x, int length) {
        auto y = std::abs(x);
        if (y < 10) {
            return formatFloat(x, length - 1);
        } else if (y < 100) {
            return formatFloat(x, length - 2);
        } else if (y < 1000) {
            return formatFloat(x, length - 3);
        } else if (y < 10000) {
            return formatFloat(x, length - 4);
        } else {
            return formatFloat(x, length - 5);
        }
    }

    class UIBase {
    public:
        explicit UIBase(juce::AudioProcessorValueTreeState &apvts)
            : state(apvts), fontSize{0.f}, styleID{1} {
        }

        UIBase(const float fSize, const size_t idx, juce::AudioProcessorValueTreeState &apvts)
            : state(apvts) {
            fontSize.store(fSize);
            styleID.store(idx);
            mainID.store(std::min(static_cast<size_t>(1), idx));
        }

        void setFontSize(const float fSize) { fontSize.store(fSize); }

        inline float getFontSize() const { return fontSize.load(); }

        void setStyle(const size_t idx) {
            styleID.store(idx);
            mainID.store(std::min(static_cast<size_t>(1), idx));
        }

        inline size_t getStyle() const { return styleID.load(); }

        inline juce::Colour getTextColor() const { return styleColors[mainID.load()].TextColor; }

        inline juce::Colour getTextInactiveColor() const { return getTextColor().withAlpha(0.5f); }

        inline juce::Colour getTextHideColor() const { return getTextColor().withAlpha(0.25f); }

        inline juce::Colour getBackgroundColor() const { return styleColors[mainID.load()].BackgroundColor; }

        inline juce::Colour getTextLightColor() const { return styleColors[mainID.load()].TextLightColor; }

        inline juce::Colour getBackgroundInactiveColor() const { return getBackgroundColor().withAlpha(0.8f); }

        inline juce::Colour getBackgroundHideColor() const { return getBackgroundColor().withAlpha(0.5f); }

        inline juce::Colour getDarkShadowColor() const { return styleColors[mainID.load()].DarkShadowColor; }

        inline juce::Colour getBrightShadowColor() const { return styleColors[mainID.load()].BrightShadowColor; }

        inline juce::Colour getExtraColor1() const { return styleColors[mainID.load()].ExtraColor1; }

        inline juce::Colour getColorMap1(const size_t idx) const {
            return styleColors[std::max(static_cast<size_t>(1), mainID.load())].ColorMap1[idx % ColorMap1Size];
        }

        inline juce::Colour getColorMap2(const size_t idx) const {
            return styleColors[mainID.load()].ColorMap2[idx % ColorMap2Size];
        }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> boxBounds,
                                                                    float cornerSize,
                                                                    const fillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics &g,
                                                          juce::Rectangle<float> boxBounds,
                                                          float cornerSize,
                                                          const fillRoundedShadowRectangleArgs &margs) const;

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                               juce::Rectangle<float> boxBounds,
                                                               float cornerSize,
                                                               const fillRoundedShadowRectangleArgs &margs) const;

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                           float cornerSize,
                                                           const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics &g,
                                                 juce::Rectangle<float> boxBounds,
                                                 float cornerSize,
                                                 const fillShadowEllipseArgs &margs) const;

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                                float cornerSize,
                                                                const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics &g,
                                                      juce::Rectangle<float> boxBounds,
                                                      float cornerSize,
                                                      const fillShadowEllipseArgs &margs) const;

        juce::Colour getColourByIdx(colourIdx idx) const;

        void setColourByIdx(const colourIdx idx, const juce::Colour colour) {
            customColours[static_cast<size_t>(idx)] = colour;
        }

        inline float getWheelSensitivity(const size_t x) const {
            return wheelSensitivity[x];
        }

        void setWheelSensitivity(const float v, const size_t x) {
            wheelSensitivity[x] = v;
        }

        size_t getRotaryStyleID() const {
            return rotaryStyleId;
        }

        juce::Slider::SliderStyle getRotaryStyle() const {
            return zlState::rotaryStyle::styles[rotaryStyleId];
        }

        void setRotaryStyleID(const size_t x) {
            rotaryStyleId = x;
        }

        float getRotaryDragSensitivity() const {
            return rotaryDragSensitivity;
        }

        void setRotaryDragSensitivity(const float x) {
            rotaryDragSensitivity = x;
        }

        juce::int64 getRefreshRateMS() const {
            return static_cast<juce::int64>(zlState::refreshRate::ms[refreshRateId.load()]);
        }

        size_t getRefreshRateID() const {
            return refreshRateId.load();
        }

        void setRefreshRateID(const size_t x) {
            refreshRateId.store(x);
        }

        float getFFTExtraTilt() const {
            return fftExtraTilt.load();
        }

        void setFFTExtraTilt(const float x) {
            fftExtraTilt.store(x);
        }

        float getFFTExtraSpeed() const {
            return fftExtraSpeed.load();
        }

        void setFFTExtraSpeed(const float x) {
            fftExtraSpeed.store(x);
        }

        float getSingleCurveThickness() const {
            return singleCurveThickness.load();
        }

        void setSingleCurveThickness(const float x) {
            singleCurveThickness.store(x);
        }

        float getSumCurveThickness() const {
            return sumCurveThickness.load();
        }

        void setSumCurveThickness(const float x) {
            sumCurveThickness.store(x);
        }

        void loadFromAPVTS();

        void saveToAPVTS();

    private:
        juce::AudioProcessorValueTreeState &state;
        std::atomic<float> fontSize{0};
        std::atomic<size_t> styleID{1}, mainID{1};
        std::array<juce::Colour, colourNum> customColours;
        std::array<float, 2> wheelSensitivity {1.f, 0.12f};
        size_t rotaryStyleId {0};
        std::atomic<size_t> refreshRateId {2};
        float rotaryDragSensitivity;
        std::atomic<float> fftExtraTilt{0.f}, fftExtraSpeed{1.f};
        std::atomic<float> singleCurveThickness{1.f}, sumCurveThickness{1.f};

        float loadPara(const std::string &id) const {
            return state.getRawParameterValue(id)->load();
        }

        void savePara(const std::string &id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }
    };
}

#endif //ZL_INTERFACE_DEFINES_H
