// Copyright (C) 2026 - zsliu98
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
    enum class Alignment {
        kLeft,
        kCenter,
        kRight,
        kLeftPadding,
        kRightPadding,
    };

    class CompactComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactComboboxLookAndFeel(UIBase& base, bool align_label = true) :
            base_(base), align_label_(align_label) {
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
                g.setColour(getPopupSurfaceColour());
                fillPopupShape(g, box_bound, corner_size,
                               !isPopupAttached() || popup_below_box_,
                               !isPopupAttached() || !popup_below_box_);
            } else if (box_alpha_ > 1e-3f) {
                g.setColour(base_.getTextColour().withAlpha(kHoverAlpha * box_alpha_));
                g.fillRoundedRectangle(box_bound, corner_size);
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
            const auto label_bound = label.getLocalBounds().toFloat();
            const auto bound = align_label_
                ? getTextBounds(label_bound, label_alignment_, label_padding_)
                : label_bound;
            g.drawText(label.getText(), bound, getJustification(label_alignment_));
        }

        void drawPopupMenuBackground(juce::Graphics& g, const int width, const int height) override {
            const auto corner_size = base_.getFontSize() * 0.375f;
            const auto box_bound = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            if (popup_uses_opaque_fallback_) {
                g.fillAll(base_.getBackgroundColour());
            }
            g.setColour(base_.getBackgroundColour().withAlpha(.95f));
            fillPopupShape(g, box_bound, corner_size,
                           !isPopupAttached() || !popup_below_box_,
                           !isPopupAttached() || popup_below_box_);
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
            if ((isHighlighted || isTicked) && isActive) {
                const auto card = area.toFloat();
                g.setColour(base_.getTextColour().withAlpha(isTicked ? kSelectedAlpha : kHoverAlpha));
                g.fillRoundedRectangle(card, base_.getFontSize() * .375f);
            }
            if (icon == nullptr) {
                g.setColour(base_.getTextColour().withAlpha(alpha));
                g.setFont(base_.getFontSize() * font_scale_);
                const auto bound = getTextBounds(area.toFloat(), item_alignment_, item_padding_);
                g.drawText(text, bound, getJustification(item_alignment_));
            } else {
                const auto fig = icon->createCopy();
                fig->replaceColour(juce::Colours::black, base_.getTextColour());
                fig->drawWithin(g, area.toFloat(), juce::RectanglePlacement::centred, alpha);
            }
        }

        int getMenuWindowFlags() override {
            return 0;
        }

        int getPopupMenuBorderSize() override {
            return 0;
        }

        inline void setFontScale(const float x) { font_scale_ = x; }

        [[nodiscard]] float getFontScale() const { return font_scale_; }

        void setOption(const juce::PopupMenu::Options& x) { option_ = x; }

        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override {
            popup_target_ = &box;
            popup_placement_known_ = false;
            popup_attached_ = false;
            popup_uses_opaque_fallback_ = false;
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

        void preparePopupMenuWindow(juce::Component& new_window) override {
            if (new_window.getParentComponent() != nullptr) {
                new_window.setOpaque(false);
            }
            popup_uses_opaque_fallback_ = new_window.isOpaque();

            if (popup_target_ == nullptr || popup_placement_known_) {
                return;
            }

            alignPopupMenuWindow(new_window);
            const auto target_bounds = popup_target_->getScreenBounds();
            const auto popup_bounds = new_window.getScreenBounds();
            const auto distance_below = std::abs(popup_bounds.getY() - target_bounds.getBottom());
            const auto distance_above = std::abs(target_bounds.getY() - popup_bounds.getBottom());
            popup_below_box_ = distance_below <= distance_above;
            popup_attached_ = std::min(distance_below, distance_above) <= 1;
            popup_placement_known_ = true;
            popup_target_->repaint();
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

        void setAlignment(const Alignment alignment, const float padding = 0.f) {
            setLabelAlignment(alignment, padding);
            setItemAlignment(alignment, padding);
        }

        void setLabelAlignment(const Alignment alignment, const float padding = 0.f) {
            label_alignment_ = alignment;
            label_padding_ = std::max(0.f, padding);
            align_label_ = true;
        }

        void setItemAlignment(const Alignment alignment, const float padding = 0.f) {
            item_alignment_ = alignment;
            item_padding_ = std::max(0.f, padding);
        }

        void setAlignLabel(const bool should_align) { align_label_ = should_align; }

        void setIcons(const std::vector<std::unique_ptr<juce::Drawable>>& icons) {
            for (size_t i = 0; i < icons.size(); ++i) {
                icons_.emplace_back(icons[i]->createCopy());
            }
        }

        void setItemSize(const int width, const int height) {
            item_width_ = width;
            item_height_ = height;
        }

        void updateTextWidth(const juce::ComboBox& box) {
            const juce::Font font{juce::FontOptions(base_.getFontSize() * font_scale_)};
            max_text_width_ = 0.f;
            const auto num_items = box.getNumItems();
            for (int i = 0; i < num_items; ++i) {
                max_text_width_ = std::max(max_text_width_,
                                           juce::GlyphArrangement::getStringWidth(font, box.getItemText(i)));
            }
        }

        [[nodiscard]] float getMaxTextWidth() const noexcept {
            return max_text_width_;
        }

    private:
        static constexpr float kHoverAlpha{.045f};
        static constexpr float kSelectedAlpha{.105f};
        static constexpr float kSurfaceTint{.125f};

        void alignPopupMenuWindow(juce::Component& window) const {
            auto* parent = window.getParentComponent();
            if (parent == nullptr) {
                return;
            }

            auto available = parent->getLocalBounds().toFloat();
            if (const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(
                    popup_target_->getScreenBounds())) {
                const auto insets = display->safeAreaInsets;
                const juce::BorderSize<float> safe_border(static_cast<float>(insets.getTop()),
                                                         static_cast<float>(insets.getLeft()),
                                                         static_cast<float>(insets.getBottom()),
                                                         static_cast<float>(insets.getRight()));
                const auto screen_area = display->userBounds.getIntersection(
                    safe_border.subtractedFrom(display->logicalBounds));
                available = available.getIntersection(parent->getLocalArea(nullptr, screen_area));
            }

            const auto popup = parent->getLocalArea(&window, window.getLocalBounds().toFloat());
            if (available.isEmpty() || popup.getWidth() > available.getWidth()) {
                return;
            }

            const auto target = parent->getLocalArea(popup_target_, popup_target_->getLocalBounds().toFloat());
            const auto x = juce::jlimit(available.getX(), available.getRight() - popup.getWidth(), target.getX());
            // JUCE restores its saved bounds when scrolling; a transform keeps this correction intact.
            window.setTransform(window.getTransform().translated(x - popup.getX(), 0.f));
        }

        [[nodiscard]] static Alignment getAlignment(const juce::Justification justification) {
            if (justification.testFlags(juce::Justification::horizontallyCentred)) {
                return Alignment::kCenter;
            }
            return justification.testFlags(juce::Justification::right) ? Alignment::kRight : Alignment::kLeft;
        }

        [[nodiscard]] static juce::Justification getJustification(const Alignment alignment) {
            switch (alignment) {
            case Alignment::kLeft:
            case Alignment::kLeftPadding:
                return juce::Justification::centredLeft;
            case Alignment::kRight:
            case Alignment::kRightPadding:
                return juce::Justification::centredRight;
            case Alignment::kCenter:
                return juce::Justification::centred;
            }
            return juce::Justification::centred;
        }

        [[nodiscard]] juce::Rectangle<float> getTextBounds(const juce::Rectangle<float> bounds,
                                                           const Alignment alignment, const float padding) const {
            if (alignment == Alignment::kLeftPadding || alignment == Alignment::kRightPadding) {
                return bounds.reduced(juce::jlimit(0.f, bounds.getWidth() * .5f, padding), 0.f);
            }
            if (max_text_width_ <= 0.f || alignment == Alignment::kCenter) {
                return bounds;
            }
            const auto inset = std::max(0.f, (bounds.getWidth() - max_text_width_) * .5f);
            return alignment == Alignment::kLeft ? bounds.withTrimmedLeft(inset) : bounds.withTrimmedRight(inset);
        }

        [[nodiscard]] juce::Colour getPopupSurfaceColour() const {
            return base_.getColourBlendedWithBackground(base_.getTextColour(), kSurfaceTint);
        }

        [[nodiscard]] bool isPopupAttached() const {
            return popup_placement_known_ && popup_attached_;
        }

        static void fillPopupShape(juce::Graphics& g, const juce::Rectangle<float> bounds,
                                   const float corner_size, const bool round_top,
                                   const bool round_bottom) {
            juce::Path path;
            path.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                     corner_size, corner_size,
                                     round_top, round_top, round_bottom, round_bottom);
            g.fillPath(path);
        }

        int item_width_{0}, item_height_{0};
        float font_scale_{1.5f}, box_alpha_{0.f};
        float max_text_width_{0.f};
        Alignment label_alignment_{Alignment::kCenter};
        Alignment item_alignment_{Alignment::kCenter};
        float label_padding_{0.f}, item_padding_{0.f};
        juce::PopupMenu::Options option_{};
        juce::Component::SafePointer<juce::ComboBox> popup_target_;
        bool popup_placement_known_{false};
        bool popup_attached_{false};
        bool popup_below_box_{true};
        bool popup_uses_opaque_fallback_{false};

        UIBase& base_;
        bool align_label_{true};

        std::vector<std::unique_ptr<juce::Drawable>> icons_;
    };
}
