// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "warning_overlay.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    WarningOverlay::WarningOverlay(zlgui::UIBase& base) :
        base_(base), background_(base), message_laf_(base),
        cancel_button_(base, "Cancel"), confirm_button_(base) {
        setOpaque(false);
        setInterceptsMouseClicks(true, true);
        setWantsKeyboardFocus(true);

        addAndMakeVisible(background_);
        message_laf_.setFontScale(zlgui::kFontHuge);
        message_laf_.setMaximumNumberOfLines(3);
        message_label_.setLookAndFeel(&message_laf_);
        message_label_.setJustificationType(juce::Justification::topLeft);
        message_label_.setAlpha(.5f);
        message_label_.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(message_label_);

        const auto configure_button = [this](zlgui::button::ClickTextButton& button,
                                             const bool is_confirm) {
            button.getLAF().setFontScale(zlgui::kFontHuge);
            button.getLAF().setJustification(juce::Justification::centred);
            button.getButton().setMouseCursor(juce::MouseCursor::PointingHandCursor);
            button.setBackgroundPainter([this, is_confirm](juce::Graphics& g,
                                                            juce::Button& painted_button,
                                                            const bool highlighted,
                                                            const bool down) {
                auto colour = is_confirm && destructive_
                                  ? base_.getColourMap1(0).withAlpha(.75f)
                                  : base_.getTextColour().withAlpha(.08f);
                if (highlighted) {
                    colour = colour.brighter(.08f);
                }
                if (down) {
                    colour = colour.brighter(.14f);
                }
                g.setColour(colour);
                g.fillRoundedRectangle(painted_button.getLocalBounds().toFloat(),
                                       base_.getFontSize() * .4f);
            });
            addAndMakeVisible(button);
        };
        configure_button(cancel_button_, false);
        configure_button(confirm_button_, true);
        cancel_button_.getButton().onClick = [this]() { hide(); };
        confirm_button_.getButton().onClick = [this]() { confirm(); };
    }

    WarningOverlay::~WarningOverlay() {
        message_label_.setLookAndFeel(nullptr);
    }

    void WarningOverlay::paint(juce::Graphics& g) {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto bound = getLocalBounds().reduced(padding);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding));
        g.setColour(base_.getBackgroundColour().withAlpha(.85f));
        g.fillPath(path);
    }

    void WarningOverlay::resized() {
        const auto padding = getPaddingSize(base_.getFontSize());
        background_.setBounds(getCardBounds().expanded(padding));
        message_label_.setBounds(getMessageBounds());
        cancel_button_.setBounds(getCancelBounds());
        confirm_button_.setBounds(getConfirmBounds());
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
        on_confirm_ = std::move(on_confirm);
        message_label_.setText(message, juce::dontSendNotification);
        cancel_button_.setVisible(true);
        confirm_button_.getButton().setButtonText(action);
        setVisible(true);
        toFront(true);
        grabKeyboardFocus();
        confirm_button_.getButton().repaint();
    }

    void WarningOverlay::showMessage(const juce::String& message) {
        show(message, "OK", false, {});
        cancel_button_.setVisible(false);
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

    void WarningOverlay::confirm() {
        auto callback = std::move(on_confirm_);
        setVisible(false);
        if (callback) {
            callback();
        }
    }
}
