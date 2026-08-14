// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "warning_overlay.hpp"
#include "../../gui/popup/popup_style.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    WarningOverlay::WarningOverlay(zlgui::UIBase& base) :
        base_(base) {
        setOpaque(false);
        setInterceptsMouseClicks(true, true);
        setWantsKeyboardFocus(true);
    }

    void WarningOverlay::paint(juce::Graphics& g) {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto bound = getLocalBounds().reduced(padding);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding));
        g.setColour(base_.getBackgroundColour().withAlpha(.85f));
        g.fillPath(path);

        const auto card = getCardBounds().toFloat();
        const auto corner = font_size * .65f;
        juce::DropShadow{base_.getDarkShadowColour().withAlpha(.85f),
                         juce::roundToInt(font_size), {0, juce::roundToInt(font_size * .24f)}}
            .drawForRectangle(g, card.toNearestInt());
        g.setColour(base_.getBackgroundColour().withAlpha(zlgui::popup::kBackgroundAlpha));
        g.fillRoundedRectangle(card, corner);
        g.setColour(base_.getTextColour().withAlpha(.12f));
        g.drawRoundedRectangle(card, corner, font_size * .08f);

        g.setColour(base_.getTextInactiveColour());
        g.setFont(juce::FontOptions{zlgui::popup::textFontSize(font_size)});
        g.drawFittedText(message_, getMessageBounds(), juce::Justification::topLeft, 3, 1.f);

        if (show_cancel_) {
            paintButton(g, getCancelBounds(), "Cancel", Target::cancel, false);
        }
        paintButton(g, getConfirmBounds(), action_, Target::confirm, destructive_);
    }

    void WarningOverlay::mouseMove(const juce::MouseEvent& event) {
        const auto target = getTarget(event.getPosition());
        if (hovered_ != target) {
            hovered_ = target;
            setMouseCursor(target == Target::none
                ? juce::MouseCursor::NormalCursor
                : juce::MouseCursor::PointingHandCursor);
            repaint();
        }
    }

    void WarningOverlay::mouseExit(const juce::MouseEvent&) {
        hovered_ = Target::none;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }

    void WarningOverlay::mouseDown(const juce::MouseEvent& event) {
        pressed_ = event.mods.isPopupMenu() ? Target::none : getTarget(event.getPosition());
        repaint();
    }

    void WarningOverlay::mouseUp(const juce::MouseEvent& event) {
        const auto target = getTarget(event.getPosition());
        if (!event.mods.isPopupMenu() && event.mouseWasClicked() && target == pressed_) {
            if (target == Target::cancel) {
                hide();
            } else if (target == Target::confirm) {
                confirm();
            }
        }
        pressed_ = Target::none;
        repaint();
    }

    bool WarningOverlay::keyPressed(const juce::KeyPress& key) {
        if (key == juce::KeyPress::escapeKey) {
            hide();
            return true;
        }
        if (key == juce::KeyPress::returnKey) {
            confirm();
            return true;
        }
        return false;
    }

    void WarningOverlay::show(const juce::String& message, const juce::String& action,
                              const bool destructive,
                              std::function<void()> on_confirm) {
        destructive_ = destructive;
        show_cancel_ = true;
        on_confirm_ = std::move(on_confirm);
        action_ = action;
        message_ = message;
        hovered_ = Target::none;
        pressed_ = Target::none;
        setVisible(true);
        toFront(true);
        grabKeyboardFocus();
        repaint();
    }

    void WarningOverlay::showMessage(const juce::String& message) {
        show(message, "OK", false, {});
        show_cancel_ = false;
        repaint();
    }

    void WarningOverlay::hide() {
        on_confirm_ = {};
        setVisible(false);
    }

    juce::Rectangle<int> WarningOverlay::getCardBounds() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * 1.5f);
        const auto available_width = juce::jmax(0, getWidth() - 2 * padding);
        const auto available_height = juce::jmax(0, getHeight() - 2 * padding);
        const auto width = juce::jmin(available_width, juce::roundToInt(base_.getFontSize() * 28.f));
        const auto height = juce::jmin(available_height, juce::roundToInt(base_.getFontSize() * 8.5f));
        return getLocalBounds().withSizeKeepingCentre(width, height);
    }

    juce::Rectangle<int> WarningOverlay::getCancelBounds() const {
        if (!show_cancel_) {
            return {};
        }
        const auto font_size = base_.getFontSize();
        const auto padding = juce::roundToInt(font_size);
        const auto button_width = juce::roundToInt(font_size * 6.5f);
        const auto button_height = juce::roundToInt(font_size * 2.15f);
        auto row = getCardBounds().reduced(padding).removeFromBottom(button_height);
        row.removeFromRight(button_width + padding / 2);
        return row.removeFromRight(button_width);
    }

    juce::Rectangle<int> WarningOverlay::getConfirmBounds() const {
        const auto font_size = base_.getFontSize();
        const auto padding = juce::roundToInt(font_size);
        const auto button_width = juce::roundToInt(font_size * 6.5f);
        const auto button_height = juce::roundToInt(font_size * 2.15f);
        auto row = getCardBounds().reduced(padding).removeFromBottom(button_height);
        return row.removeFromRight(button_width);
    }

    juce::Rectangle<int> WarningOverlay::getMessageBounds() const {
        const auto font_size = base_.getFontSize();
        const auto padding = juce::roundToInt(font_size);
        const auto button_height = juce::roundToInt(font_size * 2.15f);
        auto card = getCardBounds().reduced(padding);
        card.removeFromBottom(button_height + padding / 2);
        return card;
    }

    WarningOverlay::Target WarningOverlay::getTarget(const juce::Point<int> position) const {
        if (getCancelBounds().contains(position)) {
            return Target::cancel;
        }
        if (getConfirmBounds().contains(position)) {
            return Target::confirm;
        }
        return Target::none;
    }

    void WarningOverlay::paintButton(juce::Graphics& g, const juce::Rectangle<int> bounds,
                                     const juce::String& text, const Target target,
                                     const bool destructive) const {
        auto colour = destructive
            ? base_.getColourMap1(0).withAlpha(.75f)
            : base_.getTextColour().withAlpha(.08f);
        if (hovered_ == target) {
            colour = colour.brighter(.08f);
        }
        if (pressed_ == target) {
            colour = colour.brighter(.14f);
        }
        g.setColour(colour);
        const auto font_size = base_.getFontSize();
        g.fillRoundedRectangle(bounds.toFloat(), font_size * .4f);
        g.setColour(base_.getTextColour());
        g.setFont(juce::FontOptions{zlgui::popup::textFontSize(font_size)});
        g.drawFittedText(text, bounds.reduced(juce::roundToInt(font_size * .5f), 0),
                         juce::Justification::centred, 1);
    }

    void WarningOverlay::confirm() {
        auto callback = std::move(on_confirm_);
        setVisible(false);
        if (callback) {
            callback();
        }
    }
}
