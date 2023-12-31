// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_INTERFACE_DEFINES_H
#define ZL_INTERFACE_DEFINES_H

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    struct UIColors {
        juce::Colour TextColor = juce::Colour(87, 96, 110);
        juce::Colour BackgroundColor = juce::Colour(214, 223, 236);
        juce::Colour DarkShadowColor = juce::Colour(168, 172, 178);
        juce::Colour BrightShadowColor = juce::Colour(237, 246, 255);
        juce::Colour ExtraColor1 = juce::Colour(139, 0, 0);
    };

    inline const std::array<UIColors, 2> styleColors{
        {
            {},
            {
                .TextColor = juce::Colour(255 - 8, 255 - 9, 255 - 11),
                .BackgroundColor = juce::Colour(255 - 214, 255 - 223, 255 - 236),
                .DarkShadowColor = juce::Colour(0, 0, 0),
                .BrightShadowColor = juce::Colour(255 - 168, 255 - 172, 255 - 178),
                .ExtraColor1 = juce::Colour(255 - 139, 255, 255)
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
        bool changeMain = false, changeDark = false, changeBright = false;
        juce::Colour mainColour = BackgroundColor;
        juce::Colour darkShadowColor = DarkShadowColor;
        juce::Colour brightShadowColor = BrightShadowColor;
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
        UIBase();

        void setFontSize(const float fSize) { fontSize.store(fSize); }

        inline float getFontSize() { return fontSize.load(); }

        void setStyle(const size_t idx) { styleID.store(idx); }

        inline juce::Colour getTextColor() { return styleColors[styleID.load()].TextColor; }

        inline juce::Colour getTextInactiveColor() { return getTextColor().withAlpha(0.5f); }

        inline juce::Colour getTextHideColor() { return getTextColor().withAlpha(0.25f); }

        inline juce::Colour getBackgroundColor() { return styleColors[styleID.load()].BackgroundColor; }

        inline juce::Colour getBackgroundInactiveColor() { return getBackgroundColor().withAlpha(0.8f); }

        inline juce::Colour getBackgroundHideColor() { return getBackgroundColor().withAlpha(0.5f); }

        inline juce::Colour getDarkShadowColor() { return styleColors[styleID.load()].DarkShadowColor; }

        inline juce::Colour getBrightShadowColor() { return styleColors[styleID.load()].BrightShadowColor; }

        inline juce::Colour getExtraColor1() { return styleColors[styleID.load()].ExtraColor1; }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> boxBounds,
                                                                    float cornerSize,
                                                                    const fillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics &g,
                                                          juce::Rectangle<float> boxBounds,
                                                          float cornerSize,
                                                          const fillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                               juce::Rectangle<float> boxBounds,
                                                               float cornerSize,
                                                               const fillRoundedShadowRectangleArgs &margs);

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                           float cornerSize,
                                                           const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics &g,
                                                 juce::Rectangle<float> boxBounds,
                                                 float cornerSize,
                                                 const fillShadowEllipseArgs &margs);

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                                float cornerSize,
                                                                const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics &g,
                                                      juce::Rectangle<float> boxBounds,
                                                      float cornerSize,
                                                      const fillShadowEllipseArgs &margs);

    private:
        std::atomic<float> fontSize;
        std::atomic<size_t> styleID;
    };
}

#endif //ZL_INTERFACE_DEFINES_H
