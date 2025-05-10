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
    juce::Rectangle<float> UIBase::getRoundedShadowRectangleArea(juce::Rectangle<float> box_bounds, float corner_size,
                                                                 const FillRoundedShadowRectangleArgs &margs) {
        const auto radius = juce::jmax(juce::roundToInt(corner_size * margs.blur_radius * 1.5f), 1);
        return box_bounds.withSizeKeepingCentre(
            box_bounds.getWidth() - static_cast<float>(radius) - 1.42f * corner_size,
            box_bounds.getHeight() - static_cast<float>(radius) - 1.42f * corner_size);
    }

    juce::Rectangle<float> UIBase::fillRoundedShadowRectangle(juce::Graphics &g,
                                                              juce::Rectangle<float> box_bounds,
                                                              float corner_size,
                                                              const FillRoundedShadowRectangleArgs &margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColor().withAlpha(args.main_colour.getAlpha());
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColor();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColor();

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

    juce::Rectangle<float> UIBase::fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                                   juce::Rectangle<float> box_bounds,
                                                                   float corner_size,
                                                                   const FillRoundedShadowRectangleArgs &margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColor();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColor();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColor();

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
                                                        const FillShadowEllipseArgs &margs) {
        auto radius = juce::jmax(juce::roundToInt(corner_size * 0.75f), 1);
        if (margs.fit) {
            box_bounds = box_bounds.withSizeKeepingCentre(
                box_bounds.getWidth() - static_cast<float>(radius) - 1.5f * corner_size,
                box_bounds.getHeight() - static_cast<float>(radius) - 1.5f * corner_size);
        }
        return box_bounds;
    }


    juce::Rectangle<float> UIBase::drawShadowEllipse(juce::Graphics &g,
                                                     juce::Rectangle<float> box_bounds,
                                                     float corner_size,
                                                     const FillShadowEllipseArgs &margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColor();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColor();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColor();

        juce::Path path;
        auto radius = juce::jmax(juce::roundToInt(corner_size * 0.75f), 1);
        if (args.fit) {
            box_bounds = box_bounds.withSizeKeepingCentre(
                box_bounds.getWidth() - static_cast<float>(radius) - 1.5f * corner_size,
                box_bounds.getHeight() - static_cast<float>(radius) - 1.5f * corner_size);
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
        g.setColour(args.main_colour);
        g.fillPath(path);
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::getInnerShadowEllipseArea(juce::Rectangle<float> box_bounds,
                                                             float corner_size,
                                                             const FillShadowEllipseArgs &margs) {
        juce::ignoreUnused(margs);
        const auto radius = juce::jmax(juce::roundToInt(corner_size * 1.5f), 1);
        box_bounds = box_bounds.withSizeKeepingCentre(
            box_bounds.getWidth() - 0.75f * static_cast<float>(radius),
            box_bounds.getHeight() - 0.75f * static_cast<float>(radius));
        return box_bounds;
    }

    juce::Rectangle<float> UIBase::drawInnerShadowEllipse(juce::Graphics &g,
                                                          juce::Rectangle<float> box_bounds,
                                                          float corner_size,
                                                          const FillShadowEllipseArgs &margs) const {
        auto args = margs;
        if (!args.change_main)
            args.main_colour = getBackgroundColor();
        if (!args.change_dark)
            args.dark_shadow_color = getDarkShadowColor();
        if (!args.change_bright)
            args.bright_shadow_color = getBrightShadowColor();

        juce::Path mask;
        mask.addEllipse(box_bounds);
        g.saveState();
        g.reduceClipRegion(mask);
        g.fillAll(args.main_colour);
        auto radius = juce::jmax(juce::roundToInt(corner_size * 1.5f), 1);
        auto offset = static_cast<int>(corner_size * args.blur_radius) * 2;
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
        path.addEllipse(box_bounds);

        juce::DropShadow back_shadow(args.main_colour, radius, {0, 0});
        back_shadow.drawForPath(g, path);
        g.restoreState();
        return box_bounds;
    }

    void UIBase::loadFromAPVTS() {
        for (size_t i = 0; i < kColourNum; ++i) {
            const auto r = static_cast<juce::uint8>(state.getRawParameterValue(kColourNames[i] + "_r")->load());
            const auto g = static_cast<juce::uint8>(state.getRawParameterValue(kColourNames[i] + "_g")->load());
            const auto b = static_cast<juce::uint8>(state.getRawParameterValue(kColourNames[i] + "_b")->load());
            const auto o = static_cast<float>(state.getRawParameterValue(kColourNames[i] + "_o")->load());
            custom_colours_[i] = juce::Colour(r, g, b, o);
        }
        wheel_sensitivity_[0] = state.getRawParameterValue(zlstate::wheelSensitivity::ID)->load();
        wheel_sensitivity_[1] = state.getRawParameterValue(zlstate::wheelFineSensitivity::ID)->load();
        wheel_sensitivity_[2] = state.getRawParameterValue(zlstate::dragSensitivity::ID)->load();
        wheel_sensitivity_[3] = state.getRawParameterValue(zlstate::dragFineSensitivity::ID)->load();
        is_mouse_wheel_shift_reverse_.store(state.getRawParameterValue(zlstate::wheelShiftReverse::ID)->load() > .5f);
        rotary_style_id_ = static_cast<size_t>(state.getRawParameterValue(zlstate::rotaryStyle::ID)->load());
        rotary_drag_sensitivity_ = state.getRawParameterValue(zlstate::rotaryDragSensitivity::ID)->load();
        is_slider_double_click_open_editor_.store(loadPara(zlstate::sliderDoubleClickFunc::ID) > .5f);
        refresh_rate_id_.store(static_cast<size_t>(state.getRawParameterValue(zlstate::refreshRate::ID)->load()));
        fft_extra_tilt_.store(loadPara(zlstate::fftExtraTilt::ID));
        fft_extra_speed_.store(loadPara(zlstate::fftExtraSpeed::ID));
        single_curve_thickness_.store(loadPara(zlstate::singleCurveThickness::ID));
        sum_curve_thickness_.store(loadPara(zlstate::sumCurveThickness::ID));
        default_pass_filter_slope_.store(static_cast<int>(loadPara(zlstate::defaultPassFilterSlope::ID)));
        colour_map1_idx_ = static_cast<size_t>(loadPara(zlstate::colourMap1Idx::ID));
        colour_map2_idx_ = static_cast<size_t>(loadPara(zlstate::colourMap2Idx::ID));
        fft_order_idx_ = static_cast<int>(loadPara(zlstate::ffTOrder::ID));
        dyn_link_.store(static_cast<bool>(loadPara(zlstate::dynLink::ID)));
        rendering_engine_.store(static_cast<int>(loadPara(zlstate::renderingEngine::ID)));
        tooltip_on_ = static_cast<bool>(loadPara(zlstate::tooltipON::ID));
        lang_idx_ = static_cast<multilingual::Languages>(loadPara(zlstate::tooltipLang::ID));
    }

    void UIBase::saveToAPVTS() const {
        for (size_t i = 0; i < kColourNum; ++i) {
            const std::array<float, 4> rgbo = {
                custom_colours_[i].getFloatRed(),
                custom_colours_[i].getFloatGreen(),
                custom_colours_[i].getFloatBlue(),
                custom_colours_[i].getFloatAlpha()
            };
            const std::array<std::string, 4> ID{
                kColourNames[i] + "_r",
                kColourNames[i] + "_g",
                kColourNames[i] + "_b",
                kColourNames[i] + "_o"
            };
            for (size_t j = 0; j < 4; ++j) {
                savePara(ID[j], rgbo[j]);
            }
        }
        savePara(zlstate::wheelSensitivity::ID,
                 zlstate::wheelSensitivity::convertTo01(wheel_sensitivity_[0]));
        savePara(zlstate::wheelFineSensitivity::ID,
                 zlstate::wheelFineSensitivity::convertTo01(wheel_sensitivity_[1]));
        savePara(zlstate::dragSensitivity::ID,
                 zlstate::dragSensitivity::convertTo01(wheel_sensitivity_[2]));
        savePara(zlstate::dragFineSensitivity::ID,
                 zlstate::dragFineSensitivity::convertTo01(wheel_sensitivity_[3]));
        savePara(zlstate::wheelShiftReverse::ID,
                 zlstate::wheelShiftReverse::convertTo01(static_cast<int>(is_mouse_wheel_shift_reverse_.load())));
        savePara(zlstate::rotaryStyle::ID,
                 zlstate::rotaryStyle::convertTo01(static_cast<int>(rotary_style_id_)));
        savePara(zlstate::rotaryDragSensitivity::ID,
                 zlstate::rotaryDragSensitivity::convertTo01(rotary_drag_sensitivity_));
        savePara(zlstate::sliderDoubleClickFunc::ID, static_cast<float>(is_slider_double_click_open_editor_.load()));
        savePara(zlstate::refreshRate::ID, zlstate::refreshRate::convertTo01(static_cast<int>(refresh_rate_id_.load())));
        savePara(zlstate::fftExtraTilt::ID, zlstate::fftExtraTilt::convertTo01(fft_extra_tilt_.load()));
        savePara(zlstate::fftExtraSpeed::ID, zlstate::fftExtraSpeed::convertTo01(fft_extra_speed_.load()));
        savePara(zlstate::singleCurveThickness::ID,
                 zlstate::singleCurveThickness::convertTo01(single_curve_thickness_.load()));
        savePara(zlstate::sumCurveThickness::ID,
                 zlstate::sumCurveThickness::convertTo01(sum_curve_thickness_.load()));
        savePara(zlstate::defaultPassFilterSlope::ID,
                 zlstate::defaultPassFilterSlope::convertTo01(default_pass_filter_slope_.load()));
        savePara(zlstate::colourMap1Idx::ID, zlstate::colourMapIdx::convertTo01(static_cast<int>(colour_map1_idx_)));
        savePara(zlstate::colourMap2Idx::ID, zlstate::colourMapIdx::convertTo01(static_cast<int>(colour_map2_idx_)));
        savePara(zlstate::ffTOrder::ID, zlstate::ffTOrder::convertTo01(fft_order_idx_));
        savePara(zlstate::dynLink::ID, zlstate::dynLink::convertTo01(dyn_link_.load()));
        savePara(zlstate::renderingEngine::ID, zlstate::renderingEngine::convertTo01(rendering_engine_.load()));
        savePara(zlstate::tooltipON::ID, zlstate::tooltipON::convertTo01(static_cast<int>(tooltip_on_)));
        savePara(zlstate::tooltipLang::ID, zlstate::tooltipLang::convertTo01(lang_idx_));
    }

    void UIBase::updateActualLangIdx() {
        if (lang_idx_ == multilingual::Languages::kLang_system) {
            const auto displayed_lang = juce::SystemStats::getDisplayLanguage();
            if (displayed_lang == "zh" || displayed_lang == "chi" || displayed_lang == "zho"
                || displayed_lang.startsWith("zh-")) {
                if (displayed_lang.startsWith("zh-Hant") || displayed_lang.startsWith("zh-hant")) {
                    actual_lang_idx_ = multilingual::Languages::kLang_zh_Hant;
                } else {
                    actual_lang_idx_ = multilingual::Languages::kLang_zh_Hans;
                }
            } else if (displayed_lang == "it" || displayed_lang == "ita" ||
                       displayed_lang.startsWith("it-") || displayed_lang.startsWith("ita-")) {
                actual_lang_idx_ = multilingual::Languages::kLang_it;
            } else if (displayed_lang == "ja" || displayed_lang == "jpn" ||
                       displayed_lang.startsWith("ja-") || displayed_lang.startsWith("jpn-")) {
                actual_lang_idx_ = multilingual::Languages::kLang_ja;
            } else if (displayed_lang == "de" || displayed_lang == "deu" || displayed_lang == "ger" ||
                       displayed_lang.startsWith("de-") || displayed_lang.startsWith("deu-") ||
                       displayed_lang.startsWith("ger-")) {
                actual_lang_idx_ = multilingual::Languages::kLang_de;
            } else if (displayed_lang == "es" || displayed_lang == "spa" ||
                       displayed_lang.startsWith("es-") || displayed_lang.startsWith("spa-")) {
                actual_lang_idx_ = multilingual::Languages::kLang_es;
            } else {
                actual_lang_idx_ = multilingual::Languages::kLang_en;
            }
        } else {
            actual_lang_idx_ = lang_idx_;
        }
    }
}
