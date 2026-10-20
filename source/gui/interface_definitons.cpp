// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "interface_definitions.hpp"

namespace zlgui {
    juce::Rectangle<float> UIBase::getRoundedShadowRectangleArea(juce::Rectangle<float> box_bounds, float corner_size,
                                                                 const FillRoundedShadowRectangleArgs& margs) {
        const auto radius = juce::jmax(juce::roundToInt(corner_size * margs.blur_radius * 1.5f), 1);
        return box_bounds.withSizeKeepingCentre(
            box_bounds.getWidth() - static_cast<float>(radius) - 1.42f * corner_size,
            box_bounds.getHeight() - static_cast<float>(radius) - 1.42f * corner_size);
    }

    juce::Rectangle<float> UIBase::fillRoundedShadowRectangle(juce::Graphics& g,
                                                              juce::Rectangle<float> box_bounds,
                                                              float corner_size,
                                                              const FillRoundedShadowRectangleArgs& margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColour().withAlpha(args.main_colour.getAlpha());
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColour();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColour();

        juce::Path path;
        const auto radius = juce::jmax(juce::roundToInt(corner_size * args.blur_radius * 1.5f), 1);
        if (args.fit) {
            box_bounds = box_bounds.withSizeKeepingCentre(
                box_bounds.getWidth() - static_cast<float>(radius) - 1.42f * corner_size,
                box_bounds.getHeight() - static_cast<float>(radius) - 1.42f * corner_size);
        }
        path.addRoundedRectangle(box_bounds.getX(), box_bounds.getY(),
                                 box_bounds.getWidth(), box_bounds.getHeight(),
                                 corner_size, corner_size,
                                 args.curve_top_left, args.curve_top_right,
                                 args.curve_bottom_left, args.curve_bottom_right);
        auto offset = static_cast<int>(corner_size * args.blur_radius);
        juce::Path mask(path);
        mask.setUsingNonZeroWinding(false);
        mask.addRectangle(box_bounds.withSizeKeepingCentre(box_bounds.getWidth() + corner_size * 3,
                                                           box_bounds.getHeight() + corner_size * 3));
        g.saveState();
        g.reduceClipRegion(mask);
        if (args.draw_bright) {
            juce::DropShadow bright_shadow(args.bright_shadow_color, radius,
                                           {-offset, -offset});
            bright_shadow.drawForPath(g, path);
        }
        if (args.draw_dark) {
            juce::DropShadow dark_shadow(args.dark_shadow_color, radius,
                                         {offset, offset});
            dark_shadow.drawForPath(g, path);
        }
        g.restoreState();
        if (args.draw_main) {
            g.setColour(args.main_colour);
            g.fillPath(path);
        }
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::fillRoundedInnerShadowRectangle(juce::Graphics& g,
                                                                   juce::Rectangle<float> box_bounds,
                                                                   float corner_size,
                                                                   const FillRoundedShadowRectangleArgs& margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColour();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColour();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColour();

        juce::Path mask;
        mask.addRoundedRectangle(box_bounds.getX(), box_bounds.getY(),
                                 box_bounds.getWidth(), box_bounds.getHeight(),
                                 corner_size, corner_size,
                                 args.curve_top_left, args.curve_top_right,
                                 args.curve_bottom_left, args.curve_bottom_right);
        g.saveState();
        g.reduceClipRegion(mask);
        if (args.draw_main) {
            g.fillAll(args.main_colour);
        }
        auto offset = static_cast<int>(corner_size * args.blur_radius);
        auto radius = juce::jmax(juce::roundToInt(corner_size * args.blur_radius * 1.5f), 1);
        if (!args.flip) {
            juce::DropShadow dark_shadow(args.dark_shadow_color.withMultipliedAlpha(0.75f), radius,
                                         {-offset, -offset});
            dark_shadow.drawForPath(g, mask);
            juce::DropShadow bright_shadow(args.bright_shadow_color, radius,
                                           {offset, offset});
            bright_shadow.drawForPath(g, mask);
        } else {
            juce::DropShadow bright_shadow(args.dark_shadow_color, radius,
                                           {offset, offset});
            bright_shadow.drawForPath(g, mask);
            juce::DropShadow dark_shadow(args.bright_shadow_color.withMultipliedAlpha(0.75f), radius,
                                         {-offset, -offset});
            dark_shadow.drawForPath(g, mask);
        }
        box_bounds = box_bounds.withSizeKeepingCentre(
            box_bounds.getWidth() - 0.75f * static_cast<float>(radius),
            box_bounds.getHeight() - 0.75f * static_cast<float>(radius));
        juce::Path path;
        path.addRoundedRectangle(box_bounds.getX(), box_bounds.getY(),
                                 box_bounds.getWidth(), box_bounds.getHeight(),
                                 corner_size, corner_size,
                                 args.curve_top_left, args.curve_top_right,
                                 args.curve_bottom_left, args.curve_bottom_right);

        juce::DropShadow back_shadow(args.main_colour, radius,
                                     {0, 0});
        back_shadow.drawForPath(g, path);
        g.restoreState();
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::getShadowEllipseArea(juce::Rectangle<float> box_bounds, float corner_size,
                                                        const FillShadowEllipseArgs& margs) {
        auto radius = juce::jmax(juce::roundToInt(corner_size * 0.75f), 1);
        if (margs.fit) {
            box_bounds = box_bounds.withSizeKeepingCentre(
                box_bounds.getWidth() - static_cast<float>(radius) - 1.5f * corner_size,
                box_bounds.getHeight() - static_cast<float>(radius) - 1.5f * corner_size);
        }
        return box_bounds;
    }


    juce::Rectangle<float> UIBase::drawShadowEllipse(juce::Graphics& g,
                                                     juce::Rectangle<float> box_bounds,
                                                     float corner_size,
                                                     const FillShadowEllipseArgs& margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColour();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColour();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColour();

        juce::Path path;
        auto radius = juce::jmax(juce::roundToInt(corner_size * 0.75f), 1);
        if (args.fit) {
            box_bounds = box_bounds.reduced((static_cast<float>(radius) + 1.5f * corner_size) * .5f);
        }
        path.addEllipse(box_bounds);
        auto offset = static_cast<int>(corner_size * args.blur_radius);
        juce::Path mask;
        mask.addEllipse(box_bounds.withSizeKeepingCentre(box_bounds.getWidth() * 3, box_bounds.getHeight() * 3));
        mask.setUsingNonZeroWinding(false);
        mask.addEllipse(box_bounds);
        g.saveState();
        g.reduceClipRegion(mask);
        if (!args.flip) {
            if (args.draw_dark) {
                juce::DropShadow dark_shadow(args.dark_shadow_color, radius,
                                             {offset, offset});
                dark_shadow.drawForPath(g, path);
            }
            if (args.draw_bright) {
                juce::DropShadow bright_shadow(args.bright_shadow_color, radius,
                                               {-offset, -offset});
                bright_shadow.drawForPath(g, path);
            }
        } else {
            if (args.draw_dark) {
                juce::DropShadow dark_shadow(args.dark_shadow_color, radius,
                                             {-offset, -offset});
                dark_shadow.drawForPath(g, path);
            }
            if (args.draw_bright) {
                juce::DropShadow bright_shadow(args.bright_shadow_color, radius,
                                               {offset, offset});
                bright_shadow.drawForPath(g, path);
            }
        }
        g.restoreState();
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::getInnerShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                             const float corner_size,
                                                             const FillShadowEllipseArgs& margs) {
        juce::ignoreUnused(margs);
        const auto radius = juce::jmax(juce::roundToInt(corner_size * 1.5f), 1);
        box_bounds = box_bounds.reduced(0.375f * static_cast<float>(radius));
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::drawInnerShadowEllipse(juce::Graphics& g,
                                                          juce::Rectangle<float> box_bounds,
                                                          float corner_size,
                                                          const FillShadowEllipseArgs& margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColour();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColour();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColour();

        auto radius = juce::jmax(juce::roundToInt(corner_size * 1.5f), 1);
        auto offset = static_cast<int>(corner_size * args.blur_radius) * 2;

        juce::Path mask;
        mask.addEllipse(box_bounds);
        mask.setUsingNonZeroWinding(false);
        mask.addEllipse(box_bounds.reduced(static_cast<float>(radius + offset)));
        g.saveState();
        g.reduceClipRegion(mask);

        if (!args.flip) {
            juce::DropShadow dark_shadow(args.dark_shadow_color.withMultipliedAlpha(0.75f), radius,
                                         {-offset, -offset});
            dark_shadow.drawForPath(g, mask);
            juce::DropShadow bright_shadow(args.bright_shadow_color, radius,
                                           {offset, offset});
            bright_shadow.drawForPath(g, mask);
        } else {
            juce::DropShadow bright_shadow(args.dark_shadow_color, radius,
                                           {offset, offset});
            bright_shadow.drawForPath(g, mask);
            juce::DropShadow dark_shadow(args.bright_shadow_color.withMultipliedAlpha(0.75f), radius,
                                         {-offset, -offset});
            dark_shadow.drawForPath(g, mask);
        }
        box_bounds = box_bounds.reduced(0.375f * static_cast<float>(radius));
        juce::Path path;
        path.addEllipse(box_bounds);

        juce::DropShadow back_shadow(args.main_colour, radius, {0, 0});
        back_shadow.drawForPath(g, path);
        g.restoreState();
        return box_bounds;
    }
}
