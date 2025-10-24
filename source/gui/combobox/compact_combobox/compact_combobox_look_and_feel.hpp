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
        explicit CompactComboboxLookAndFeel(UIBase& base) :
            base_(base) {
            setColour(juce::PopupMenu::backgroundColourId, base_.getBackgroundInactiveColour());
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox& box) override {
            juce::ignoreUnused(width, height);
            const auto box_bound = juce::Rectangle<float>(0, 0,
                                                          static_cast<float>(width),
                                                          static_cast<float>(height));
            const auto corner_size = base_.getFontSize() * 0.375f;
            if (isButtonDown || box.isPopupActive()) {
                g.setColour(base_.getTextInactiveColour());
                g.fillRoundedRectangle(box_bound, corner_size);
            } else if (box_alpha_ > 1e-3f) {
                base_.fillRoundedInnerShadowRectangle(g, box_bound, corner_size,
                                                      {
                                                          .blur_radius = 0.45f, .flip = true,
                                                          .main_colour = base_.getBackgroundColour().
                                                                               withMultipliedAlpha(
                                                                                   juce::jlimit(.25f, .5f, box_alpha_)),
                                                          .dark_shadow_color = base_.getDarkShadowColour().
                                                          withMultipliedAlpha(box_alpha_),
                                                          .bright_shadow_color = base_.getBrightShadowColour().
                                                          withMultipliedAlpha(box_alpha_),
                                                          .change_main = true, .change_dark = true,
                                                          .change_bright = true
                                                      });
            }
            if (!icons_.empty() && box.getSelectedItemIndex() >= 0) {
                const auto fig = icons_[static_cast<size_t>(box.getSelectedItemIndex())]->createCopy();
                fig->replaceColour(juce::Colours::black, base_.getTextColour());
                fig->drawWithin(g, box.getLocalBounds().toFloat(), juce::RectanglePlacement::centred, 1.f);
            }
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override {
            label.setBounds(box.getLocalBounds());
        }

        void drawLabel(juce::Graphics& g, juce::Label& label) override {
            g.setColour(base_.getTextColour());
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(label.getText(), label.getLocalBounds(), label_justification_);
        }

        void drawPopupMenuBackground(juce::Graphics& g, const int width, const int height) override {
            const auto corner_size = base_.getFontSize() * 0.375f;
            const auto box_bound = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            g.fillRoundedRectangle(box_bound, corner_size);
        }

        void getIdealPopupMenuItemSize(const juce::String& text, const bool isSeparator, int standardMenuItemHeight,
                                       int& ideal_width, int& ideal_height) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            ideal_width = item_width_;
            ideal_height = item_height_;
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* const textColourToUse) override {
            juce::ignoreUnused(isSeparator, hasSubMenu, shortcutKeyText, textColourToUse);
            float alpha;
            if ((isHighlighted || isTicked) && isActive) {
                alpha = 1.0;
            } else if (!isActive) {
                alpha = .125f;
            } else {
                alpha = .5f;
            }
            if (icon == nullptr) {
                g.setColour(base_.getTextColour().withAlpha(alpha));
                g.setFont(base_.getFontSize() * font_scale_);
                const auto bound = area.toFloat().reduced(padding_, 0.f);
                g.drawText(text, bound, item_justification_);
            } else {
                const auto fig = icon->createCopy();
                fig->replaceColour(juce::Colours::black, base_.getTextColour());
                fig->drawWithin(g, area.toFloat(), juce::RectanglePlacement::centred, alpha);
            }
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return 0;
        }

        inline void setFontScale(const float x) { font_scale_ = x; }

        float getFontScale() const { return font_scale_; }

        void setOption(const juce::PopupMenu::Options& x) { option_ = x; }

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

        void drawPopupMenuColumnSeparatorWithOptions(juce::Graphics& g, const juce::Rectangle<int>& bounds,
            const juce::PopupMenu::Options&) override {
            auto bound = bounds.toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() * .5f, bound.getHeight());

            juce::ColourGradient gradient;
            gradient.point1 = bound.getTopLeft();
            gradient.point2 = bound.getBottomLeft();
            gradient.isRadial = false;
            const auto colour1 = base_.getColourBlendedWithBackground(base_.getTextColour(), .1f);
            const auto colour2 = base_.getColourBlendedWithBackground(base_.getTextColour(), .25f);
            gradient.addColour(0.f, colour1);
            gradient.addColour(.5f, colour2);
            gradient.addColour(1.f, colour1);
            g.setGradientFill(gradient);
            g.fillRect(bound);
        }

        int getPopupMenuColumnSeparatorWidthWithOptions(const juce::PopupMenu::Options&) override {
            return static_cast<int>(base_.getFontSize() * .4f);
        }

        void setBoxAlpha(const float x) { box_alpha_ = x; }

        void setLabelJustification(const juce::Justification j) { label_justification_ = j; }

        void setItemJustification(const juce::Justification j) { item_justification_ = j; }

        void setPadding(const float padding) { padding_ = padding; }

        void setIcons(const std::vector<std::unique_ptr<juce::Drawable>>& icons) {
            for (size_t i = 0; i < icons.size(); ++i) {
                icons_.emplace_back(icons[i]->createCopy());
            }
        }

        void setItemSize(const int width, const int height) {
            item_width_ = width;
            item_height_ = height;
        }

    private:
        int item_width_{0}, item_height_{0};
        float font_scale_{1.5f}, box_alpha_{0.f};
        float padding_{0.f};
        juce::Justification label_justification_{juce::Justification::centred};
        juce::Justification item_justification_{juce::Justification::centred};
        juce::PopupMenu::Options option_{};

        UIBase& base_;

        std::vector<std::unique_ptr<juce::Drawable>> icons_;
    };
}
