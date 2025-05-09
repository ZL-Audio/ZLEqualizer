// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "interface_definitions.hpp"
#include "../state/state_definitions.hpp"

namespace zlgui {
    juce::Rectangle<float> UIBase::getRoundedShadowRectangleArea(juce::Rectangle<float> boxBounds, float cornerSize,
                                                                 const fillRoundedShadowRectangleArgs &margs) {
        const auto radius = juce::jmax(juce::roundToInt(cornerSize * margs.blurRadius * 1.5f), 1);
        return boxBounds.withSizeKeepingCentre(
            boxBounds.getWidth() - static_cast<float>(radius) - 1.42f * cornerSize,
            boxBounds.getHeight() - static_cast<float>(radius) - 1.42f * cornerSize);
    }

    juce::Rectangle<float> UIBase::fillRoundedShadowRectangle(juce::Graphics &g,
                                                              juce::Rectangle<float> boxBounds,
                                                              float cornerSize,
                                                              const fillRoundedShadowRectangleArgs &margs) const {
        auto args = margs;
        if (!args.changeMain)
            args.mainColour = getBackgroundColor().withAlpha(args.mainColour.getAlpha());
        if (!args.changeDark)
            args.darkShadowColor = getDarkShadowColor();
        if (!args.changeBright)
            args.brightShadowColor = getBrightShadowColor();

        juce::Path path;
        const auto radius = juce::jmax(juce::roundToInt(cornerSize * args.blurRadius * 1.5f), 1);
        if (args.fit) {
            boxBounds = boxBounds.withSizeKeepingCentre(
                boxBounds.getWidth() - static_cast<float>(radius) - 1.42f * cornerSize,
                boxBounds.getHeight() - static_cast<float>(radius) - 1.42f * cornerSize);
        }
        path.addRoundedRectangle(boxBounds.getX(), boxBounds.getY(),
                                 boxBounds.getWidth(), boxBounds.getHeight(),
                                 cornerSize, cornerSize,
                                 args.curveTopLeft, args.curveTopRight,
                                 args.curveBottomLeft, args.curveBottomRight);
        auto offset = static_cast<int>(cornerSize * args.blurRadius);
        juce::Path mask(path);
        mask.setUsingNonZeroWinding(false);
        mask.addRectangle(boxBounds.withSizeKeepingCentre(boxBounds.getWidth() + cornerSize * 3,
                                                          boxBounds.getHeight() + cornerSize * 3));
        g.saveState();
        g.reduceClipRegion(mask);
        if (args.drawBright) {
            juce::DropShadow brightShadow(args.brightShadowColor, radius,
                                          {-offset, -offset});
            brightShadow.drawForPath(g, path);
        }
        if (args.drawDark) {
            juce::DropShadow darkShadow(args.darkShadowColor, radius,
                                        {offset, offset});
            darkShadow.drawForPath(g, path);
        }
        g.restoreState();
        if (args.drawMain) {
            g.setColour(args.mainColour);
            g.fillPath(path);
        }
        return boxBounds;
    }

    juce::Rectangle<float> UIBase::fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                                   juce::Rectangle<float> boxBounds,
                                                                   float cornerSize,
                                                                   const fillRoundedShadowRectangleArgs &margs) const {
        auto args = margs;
        if (!args.changeMain)
            args.mainColour = getBackgroundColor();
        if (!args.changeDark)
            args.darkShadowColor = getDarkShadowColor();
        if (!args.changeBright)
            args.brightShadowColor = getBrightShadowColor();

        juce::Path mask;
        mask.addRoundedRectangle(boxBounds.getX(), boxBounds.getY(),
                                 boxBounds.getWidth(), boxBounds.getHeight(),
                                 cornerSize, cornerSize,
                                 args.curveTopLeft, args.curveTopRight,
                                 args.curveBottomLeft, args.curveBottomRight);
        g.saveState();
        g.reduceClipRegion(mask);
        if (args.drawMain) {
            g.fillAll(args.mainColour);
        }
        auto offset = static_cast<int>(cornerSize * args.blurRadius);
        auto radius = juce::jmax(juce::roundToInt(cornerSize * args.blurRadius * 1.5f), 1);
        if (!args.flip) {
            juce::DropShadow darkShadow(args.darkShadowColor.withMultipliedAlpha(0.75f), radius,
                                        {-offset, -offset});
            darkShadow.drawForPath(g, mask);
            juce::DropShadow brightShadow(args.brightShadowColor, radius,
                                          {offset, offset});
            brightShadow.drawForPath(g, mask);
        } else {
            juce::DropShadow brightShadow(args.darkShadowColor, radius,
                                          {offset, offset});
            brightShadow.drawForPath(g, mask);
            juce::DropShadow darkShadow(args.brightShadowColor.withMultipliedAlpha(0.75f), radius,
                                        {-offset, -offset});
            darkShadow.drawForPath(g, mask);
        }
        boxBounds = boxBounds.withSizeKeepingCentre(
            boxBounds.getWidth() - 0.75f * static_cast<float>(radius),
            boxBounds.getHeight() - 0.75f * static_cast<float>(radius));
        juce::Path path;
        path.addRoundedRectangle(boxBounds.getX(), boxBounds.getY(),
                                 boxBounds.getWidth(), boxBounds.getHeight(),
                                 cornerSize, cornerSize,
                                 args.curveTopLeft, args.curveTopRight,
                                 args.curveBottomLeft, args.curveBottomRight);

        juce::DropShadow backShadow(args.mainColour, radius,
                                    {0, 0});
        backShadow.drawForPath(g, path);
        g.restoreState();
        return boxBounds;
    }

    juce::Rectangle<float> UIBase::getShadowEllipseArea(juce::Rectangle<float> boxBounds, float cornerSize,
                                                        const fillShadowEllipseArgs &margs) {
        auto radius = juce::jmax(juce::roundToInt(cornerSize * 0.75f), 1);
        if (margs.fit) {
            boxBounds = boxBounds.withSizeKeepingCentre(
                boxBounds.getWidth() - static_cast<float>(radius) - 1.5f * cornerSize,
                boxBounds.getHeight() - static_cast<float>(radius) - 1.5f * cornerSize);
        }
        return boxBounds;
    }


    juce::Rectangle<float> UIBase::drawShadowEllipse(juce::Graphics &g,
                                                     juce::Rectangle<float> boxBounds,
                                                     float cornerSize,
                                                     const fillShadowEllipseArgs &margs) const {
        auto args = margs;
        if (!args.changeMain)
            args.mainColour = getBackgroundColor();
        if (!args.changeDark)
            args.darkShadowColor = getDarkShadowColor();
        if (!args.changeBright)
            args.brightShadowColor = getBrightShadowColor();

        juce::Path path;
        auto radius = juce::jmax(juce::roundToInt(cornerSize * 0.75f), 1);
        if (args.fit) {
            boxBounds = boxBounds.withSizeKeepingCentre(
                boxBounds.getWidth() - static_cast<float>(radius) - 1.5f * cornerSize,
                boxBounds.getHeight() - static_cast<float>(radius) - 1.5f * cornerSize);
        }
        path.addEllipse(boxBounds);
        auto offset = static_cast<int>(cornerSize * args.blurRadius);
        juce::Path mask;
        mask.addEllipse(boxBounds.withSizeKeepingCentre(boxBounds.getWidth() * 3, boxBounds.getHeight() * 3));
        mask.setUsingNonZeroWinding(false);
        mask.addEllipse(boxBounds);
        g.saveState();
        g.reduceClipRegion(mask);
        if (!args.flip) {
            if (args.drawDark) {
                juce::DropShadow darkShadow(args.darkShadowColor, radius,
                                            {offset, offset});
                darkShadow.drawForPath(g, path);
            }
            if (args.drawBright) {
                juce::DropShadow brightShadow(args.brightShadowColor, radius,
                                              {-offset, -offset});
                brightShadow.drawForPath(g, path);
            }
        } else {
            if (args.drawDark) {
                juce::DropShadow darkShadow(args.darkShadowColor, radius,
                                            {-offset, -offset});
                darkShadow.drawForPath(g, path);
            }
            if (args.drawBright) {
                juce::DropShadow brightShadow(args.brightShadowColor, radius,
                                              {offset, offset});
                brightShadow.drawForPath(g, path);
            }
        }
        g.restoreState();
        g.setColour(args.mainColour);
        g.fillPath(path);
        return boxBounds;
    }

    juce::Rectangle<float> UIBase::getInnerShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                             float cornerSize,
                                                             const fillShadowEllipseArgs &margs) {
        juce::ignoreUnused(margs);
        auto radius = juce::jmax(juce::roundToInt(cornerSize * 1.5f), 1);
        boxBounds = boxBounds.withSizeKeepingCentre(
            boxBounds.getWidth() - 0.75f * static_cast<float>(radius),
            boxBounds.getHeight() - 0.75f * static_cast<float>(radius));
        return boxBounds;
    }

    juce::Rectangle<float> UIBase::drawInnerShadowEllipse(juce::Graphics &g,
                                                          juce::Rectangle<float> boxBounds,
                                                          float cornerSize,
                                                          const fillShadowEllipseArgs &margs) const {
        auto args = margs;
        if (!args.changeMain)
            args.mainColour = getBackgroundColor();
        if (!args.changeDark)
            args.darkShadowColor = getDarkShadowColor();
        if (!args.changeBright)
            args.brightShadowColor = getBrightShadowColor();

        juce::Path mask;
        mask.addEllipse(boxBounds);
        g.saveState();
        g.reduceClipRegion(mask);
        g.fillAll(args.mainColour);
        auto radius = juce::jmax(juce::roundToInt(cornerSize * 1.5f), 1);
        auto offset = static_cast<int>(cornerSize * args.blurRadius) * 2;
        if (!args.flip) {
            juce::DropShadow darkShadow(args.darkShadowColor.withMultipliedAlpha(0.75f), radius,
                                        {-offset, -offset});
            darkShadow.drawForPath(g, mask);
            juce::DropShadow brightShadow(args.brightShadowColor, radius,
                                          {offset, offset});
            brightShadow.drawForPath(g, mask);
        } else {
            juce::DropShadow brightShadow(args.darkShadowColor, radius,
                                          {offset, offset});
            brightShadow.drawForPath(g, mask);
            juce::DropShadow darkShadow(args.brightShadowColor.withMultipliedAlpha(0.75f), radius,
                                        {-offset, -offset});
            darkShadow.drawForPath(g, mask);
        }
        boxBounds = boxBounds.withSizeKeepingCentre(
            boxBounds.getWidth() - 0.75f * static_cast<float>(radius),
            boxBounds.getHeight() - 0.75f * static_cast<float>(radius));
        juce::Path path;
        path.addEllipse(boxBounds);

        juce::DropShadow backShadow(args.mainColour, radius, {0, 0});
        backShadow.drawForPath(g, path);
        g.restoreState();
        return boxBounds;
    }

    void UIBase::loadFromAPVTS() {
        for (size_t i = 0; i < colourNum; ++i) {
            const auto r = static_cast<juce::uint8>(state.getRawParameterValue(colourNames[i] + "_r")->load());
            const auto g = static_cast<juce::uint8>(state.getRawParameterValue(colourNames[i] + "_g")->load());
            const auto b = static_cast<juce::uint8>(state.getRawParameterValue(colourNames[i] + "_b")->load());
            const auto o = static_cast<float>(state.getRawParameterValue(colourNames[i] + "_o")->load());
            customColours[i] = juce::Colour(r, g, b, o);
        }
        wheelSensitivity[0] = state.getRawParameterValue(zlstate::wheelSensitivity::ID)->load();
        wheelSensitivity[1] = state.getRawParameterValue(zlstate::wheelFineSensitivity::ID)->load();
        wheelSensitivity[2] = state.getRawParameterValue(zlstate::dragSensitivity::ID)->load();
        wheelSensitivity[3] = state.getRawParameterValue(zlstate::dragFineSensitivity::ID)->load();
        isMouseWheelShiftReverse.store(state.getRawParameterValue(zlstate::wheelShiftReverse::ID)->load() > .5f);
        rotaryStyleId = static_cast<size_t>(state.getRawParameterValue(zlstate::rotaryStyle::ID)->load());
        rotaryDragSensitivity = state.getRawParameterValue(zlstate::rotaryDragSensitivity::ID)->load();
        isSliderDoubleClickOpenEditor.store(loadPara(zlstate::sliderDoubleClickFunc::ID) > .5f);
        refreshRateId.store(static_cast<size_t>(state.getRawParameterValue(zlstate::refreshRate::ID)->load()));
        fftExtraTilt.store(loadPara(zlstate::fftExtraTilt::ID));
        fftExtraSpeed.store(loadPara(zlstate::fftExtraSpeed::ID));
        singleCurveThickness.store(loadPara(zlstate::singleCurveThickness::ID));
        sumCurveThickness.store(loadPara(zlstate::sumCurveThickness::ID));
        defaultPassFilterSlope.store(static_cast<int>(loadPara(zlstate::defaultPassFilterSlope::ID)));
        cMap1Idx = static_cast<size_t>(loadPara(zlstate::colourMap1Idx::ID));
        cMap2Idx = static_cast<size_t>(loadPara(zlstate::colourMap2Idx::ID));
        fftOrderIdx = static_cast<int>(loadPara(zlstate::ffTOrder::ID));
        dynLink.store(static_cast<bool>(loadPara(zlstate::dynLink::ID)));
        renderingEngine.store(static_cast<int>(loadPara(zlstate::renderingEngine::ID)));
        tooltipON = static_cast<bool>(loadPara(zlstate::tooltipON::ID));
        langIdx = static_cast<multilingual::languages>(loadPara(zlstate::tooltipLang::ID));
    }

    void UIBase::saveToAPVTS() const {
        for (size_t i = 0; i < colourNum; ++i) {
            const std::array<float, 4> rgbo = {
                customColours[i].getFloatRed(),
                customColours[i].getFloatGreen(),
                customColours[i].getFloatBlue(),
                customColours[i].getFloatAlpha()
            };
            const std::array<std::string, 4> ID{
                colourNames[i] + "_r",
                colourNames[i] + "_g",
                colourNames[i] + "_b",
                colourNames[i] + "_o"
            };
            for (size_t j = 0; j < 4; ++j) {
                savePara(ID[j], rgbo[j]);
            }
        }
        savePara(zlstate::wheelSensitivity::ID,
                 zlstate::wheelSensitivity::convertTo01(wheelSensitivity[0]));
        savePara(zlstate::wheelFineSensitivity::ID,
                 zlstate::wheelFineSensitivity::convertTo01(wheelSensitivity[1]));
        savePara(zlstate::dragSensitivity::ID,
                 zlstate::dragSensitivity::convertTo01(wheelSensitivity[2]));
        savePara(zlstate::dragFineSensitivity::ID,
                 zlstate::dragFineSensitivity::convertTo01(wheelSensitivity[3]));
        savePara(zlstate::wheelShiftReverse::ID,
                 zlstate::wheelShiftReverse::convertTo01(static_cast<int>(isMouseWheelShiftReverse.load())));
        savePara(zlstate::rotaryStyle::ID,
                 zlstate::rotaryStyle::convertTo01(static_cast<int>(rotaryStyleId)));
        savePara(zlstate::rotaryDragSensitivity::ID,
                 zlstate::rotaryDragSensitivity::convertTo01(rotaryDragSensitivity));
        savePara(zlstate::sliderDoubleClickFunc::ID, static_cast<float>(isSliderDoubleClickOpenEditor.load()));
        savePara(zlstate::refreshRate::ID, zlstate::refreshRate::convertTo01(static_cast<int>(refreshRateId.load())));
        savePara(zlstate::fftExtraTilt::ID, zlstate::fftExtraTilt::convertTo01(fftExtraTilt.load()));
        savePara(zlstate::fftExtraSpeed::ID, zlstate::fftExtraSpeed::convertTo01(fftExtraSpeed.load()));
        savePara(zlstate::singleCurveThickness::ID,
                 zlstate::singleCurveThickness::convertTo01(singleCurveThickness.load()));
        savePara(zlstate::sumCurveThickness::ID,
                 zlstate::sumCurveThickness::convertTo01(sumCurveThickness.load()));
        savePara(zlstate::defaultPassFilterSlope::ID,
                 zlstate::defaultPassFilterSlope::convertTo01(defaultPassFilterSlope.load()));
        savePara(zlstate::colourMap1Idx::ID, zlstate::colourMapIdx::convertTo01(static_cast<int>(cMap1Idx)));
        savePara(zlstate::colourMap2Idx::ID, zlstate::colourMapIdx::convertTo01(static_cast<int>(cMap2Idx)));
        savePara(zlstate::ffTOrder::ID, zlstate::ffTOrder::convertTo01(fftOrderIdx));
        savePara(zlstate::dynLink::ID, zlstate::dynLink::convertTo01(dynLink.load()));
        savePara(zlstate::renderingEngine::ID, zlstate::renderingEngine::convertTo01(renderingEngine.load()));
        savePara(zlstate::tooltipON::ID, zlstate::tooltipON::convertTo01(static_cast<int>(tooltipON)));
        savePara(zlstate::tooltipLang::ID, zlstate::tooltipLang::convertTo01(langIdx));
    }

    void UIBase::updateActualLangIdx() {
        if (langIdx == multilingual::languages::lang_system) {
            const auto displayedLang = juce::SystemStats::getDisplayLanguage();
            if (displayedLang == "zh" || displayedLang == "chi" || displayedLang == "zho"
                || displayedLang.startsWith("zh-")) {
                if (displayedLang.startsWith("zh-Hant") || displayedLang.startsWith("zh-hant")) {
                    actualLangIdx = multilingual::languages::lang_zh_Hant;
                } else {
                    actualLangIdx = multilingual::languages::lang_zh_Hans;
                }
            } else if (displayedLang == "it" || displayedLang == "ita" ||
                       displayedLang.startsWith("it-") || displayedLang.startsWith("ita-")) {
                actualLangIdx = multilingual::languages::lang_it;
            } else if (displayedLang == "ja" || displayedLang == "jpn" ||
                       displayedLang.startsWith("ja-") || displayedLang.startsWith("jpn-")) {
                actualLangIdx = multilingual::languages::lang_ja;
            } else if (displayedLang == "de" || displayedLang == "deu" || displayedLang == "ger" ||
                       displayedLang.startsWith("de-") || displayedLang.startsWith("deu-") ||
                       displayedLang.startsWith("ger-")) {
                actualLangIdx = multilingual::languages::lang_de;
            } else if (displayedLang == "es" || displayedLang == "spa" ||
                       displayedLang.startsWith("es-") || displayedLang.startsWith("spa-")) {
                actualLangIdx = multilingual::languages::lang_es;
            } else {
                actualLangIdx = multilingual::languages::lang_en;
            }
        } else {
            actualLangIdx = langIdx;
        }
    }
}
