// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "interface_definitions.hpp"

namespace zlInterface {
    UIBase::UIBase() : fontSize{0.f}, styleID{1} {
    }

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
            args.mainColour = getBackgroundColor().withAlpha(args.mainColour.getAlpha());
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
            args.mainColour = getBackgroundColor().withAlpha(args.mainColour.getAlpha());
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
            args.mainColour = getBackgroundColor().withAlpha(args.mainColour.getAlpha());
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
}
