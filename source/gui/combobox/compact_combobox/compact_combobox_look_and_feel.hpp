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

namespace zlgui::combobox {
    class CompactComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactComboboxLookAndFeel(UIBase& base) : base_(base) {
            setColour(juce::PopupMenu::backgroundColourId, base_.getBackgroundInactiveColor());
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox& box) override {
            juce::ignoreUnused(width, height);
            const auto boxBounds = juce::Rectangle<float>(0, 0,
                                                          static_cast<float>(width),
                                                          static_cast<float>(height));
            const auto cornerSize = base_.getFontSize() * 0.375f;
            if (isButtonDown || box.isPopupActive()) {
                g.setColour(base_.getTextInactiveColor());
                g.fillRoundedRectangle(boxBounds, cornerSize);
            } else if (box_alpha_ > 1e-3f) {
                base_.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize,
                                                      {
                                                          .blur_radius = 0.45f, .flip = true,
                                                          .main_colour = base_.getBackgroundColor().
                                                                               withMultipliedAlpha(
                                                                                   juce::jlimit(.25f, .5f, box_alpha_)),
                                                          .dark_shadow_color = base_.getDarkShadowColor().
                                                          withMultipliedAlpha(box_alpha_),
                                                          .bright_shadow_color = base_.getBrightShadowColor().
                                                          withMultipliedAlpha(box_alpha_),
                                                          .change_main = true, .change_dark = true,
                                                          .change_bright = true
                                                      });
            }
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override {
            label.setBounds(box.getLocalBounds());
        }

        void drawLabel(juce::Graphics& g, juce::Label& label) override {
            g.setColour(base_.getTextColor());
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(label.getText(), label.getLocalBounds(), label_justification_);
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override {
            const auto cornerSize = base_.getFontSize() * 0.375f;
            const auto boxBounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            base_.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blur_radius = 0.45f, .flip = true});
        }

        void getIdealPopupMenuItemSize(const juce::String& text, const bool isSeparator, int standardMenuItemHeight,
                                       int& idealWidth, int& idealHeight) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            idealWidth = static_cast<int>(0);
            idealHeight = static_cast<int>(base_.getFontSize() * font_scale_ * 1.2f);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* const textColourToUse) override {
            juce::ignoreUnused(isSeparator, hasSubMenu, shortcutKeyText, icon, textColourToUse);
            if ((isHighlighted || isTicked) && isActive) {
                g.setColour(base_.getTextColor());
            } else if (!isActive) {
                g.setColour(base_.getTextInactiveColor().withMultipliedAlpha(.25f));
            } else {
                g.setColour(base_.getTextInactiveColor());
            }
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(text, area, item_justification_);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return 0;
        }

        inline void setFontScale(const float x) { font_scale_ = x; }

        void setOption(const juce::PopupMenu::Options& x) {
            option_ = x;
        }

        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override {
            auto option = option_;
            if (option.getParentComponent() == nullptr) {
                if (juce::JUCEApplicationBase::isStandaloneApp()) {
                    option = option.withParentComponent(box.getTopLevelComponent());
                } else {
                    option = option.withParentComponent(box.getTopLevelComponent()->getChildComponent(0));
                }
            }
            if (option.getMinimumWidth() == 0) {
                option = option.withMinimumWidth(box.getWidth());
            }
            return option.withTargetComponent(&box)
                         .withInitiallySelectedItem(box.getSelectedId())
                         .withStandardItemHeight(label.getHeight());
        }

        void setBoxAlpha(const float x) { box_alpha_ = x; }

        void setLabelJustification(const juce::Justification j) { label_justification_ = j; }

        void setItemJustification(const juce::Justification j) { item_justification_ = j; }

    private:
        float font_scale_{1.5f}, box_alpha_{0.f};
        juce::Justification label_justification_{juce::Justification::centred};
        juce::Justification item_justification_{juce::Justification::centred};
        juce::PopupMenu::Options option_{};

        UIBase& base_;
    };
}
