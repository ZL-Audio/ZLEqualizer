// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlgui {
    class CompactComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        // rounded menu box
        explicit CompactComboboxLookAndFeel(UIBase &base) : ui_base_(base) {
            setColour(juce::PopupMenu::backgroundColourId, ui_base_.getBackgroundInactiveColor());
        }

        void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox &box) override {
            juce::ignoreUnused(width, height);
            const auto boxBounds = juce::Rectangle<float>(0, 0,
                                                          static_cast<float>(width),
                                                          static_cast<float>(height));
            const auto cornerSize = ui_base_.getFontSize() * 0.375f;
            if (isButtonDown || box.isPopupActive()) {
                g.setColour(ui_base_.getTextInactiveColor());
                g.fillRoundedRectangle(boxBounds, cornerSize);
            } else {
                ui_base_.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize,
                                                       {
                                                           .blur_radius = 0.45f, .flip = true,
                                                           .main_colour = ui_base_.getBackgroundColor().
                                                           withMultipliedAlpha(
                                                               juce::jlimit(.25f, .5f, box_alpha_.load())),
                                                           .dark_shadow_color = ui_base_.getDarkShadowColor().
                                                           withMultipliedAlpha(box_alpha_.load()),
                                                           .bright_shadow_color = ui_base_.getBrightShadowColor().
                                                           withMultipliedAlpha(box_alpha_.load()),
                                                           .change_main = true, .change_dark = true, .change_bright = true
                                                       });
            }
        }

        void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
            label.setBounds(box.getLocalBounds());
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (editable_.load()) {
                g.setColour(ui_base_.getTextColor());
            } else {
                g.setColour(ui_base_.getTextInactiveColor());
            }
            g.setFont(ui_base_.getFontSize() * font_scale_);
            g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
        }

        void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override {
            const auto cornerSize = ui_base_.getFontSize() * 0.375f;
            const auto boxBounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            ui_base_.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blur_radius = 0.45f, .flip = true});
        }

        void getIdealPopupMenuItemSize(const juce::String &text, const bool isSeparator, int standardMenuItemHeight,
                                       int &idealWidth, int &idealHeight) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            idealWidth = static_cast<int>(0);
            idealHeight = static_cast<int>(ui_base_.getFontSize() * font_scale_ * 1.2f);
        }

        void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String &text,
                               const juce::String &shortcutKeyText, const juce::Drawable *icon,
                               const juce::Colour *const textColourToUse) override {
            juce::ignoreUnused(isSeparator, hasSubMenu, shortcutKeyText, icon, textColourToUse);
            if ((isHighlighted || isTicked) && isActive && editable_) {
                g.setColour(ui_base_.getTextColor());
            } else if (!isActive) {
                g.setColour(ui_base_.getTextInactiveColor().withMultipliedAlpha(.25f));
            } else {
                g.setColour(ui_base_.getTextInactiveColor());
            }
            if (ui_base_.getFontSize() > 0) {
                g.setFont(ui_base_.getFontSize() * font_scale_);
            } else {
                g.setFont(static_cast<float>(area.getHeight()) * 0.35f);
            }
            g.drawText(text, area, juce::Justification::centred);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return juce::roundToInt(ui_base_.getFontSize() * 0.125f);
        }

        inline void setEditable(const bool f) { editable_.store(f); }

        inline void setBoxAlpha(const float x) { box_alpha_.store(x); }

        inline void setFontScale(const float x) { font_scale_.store(x); }

        inline float getBoxAlpha() const { return box_alpha_.load(); }

        void setOption(const juce::PopupMenu::Options &x) { option_ = x; }

        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox &box, juce::Label &label) override {
            if (!juce::JUCEApplicationBase::isStandaloneApp()) {
                return option_.withParentComponent(box.getTopLevelComponent()->getChildComponent(0))
                        .withTargetComponent(&box)
                        .withItemThatMustBeVisible(box.getSelectedId())
                        .withInitiallySelectedItem(box.getSelectedId())
                        .withMinimumWidth(box.getWidth())
                        .withMaximumNumColumns(1)
                        .withStandardItemHeight(label.getHeight());
            } else {
                return option_.withParentComponent(box.getTopLevelComponent())
                        .withTargetComponent(&box)
                        .withItemThatMustBeVisible(box.getSelectedId())
                        .withInitiallySelectedItem(box.getSelectedId())
                        .withMinimumWidth(box.getWidth())
                        .withMaximumNumColumns(1)
                        .withStandardItemHeight(label.getHeight());
            }
        }

    private:
        std::atomic<bool> editable_ = true;
        std::atomic<float> box_alpha_{0.f}, font_scale_{1.5f};
        juce::PopupMenu::Options option_{};

        UIBase &ui_base_;
    };
}
