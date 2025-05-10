// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_linear_slider.hpp"

namespace zlgui {
    CompactLinearSlider::CompactLinearSlider(const juce::String &label_text, UIBase &base,
                                             const multilingual::Labels label_idx)
        : ui_base_(base),
          slider_look_and_feel_(base), name_look_and_feel_(base), text_look_and_feel_(base),
          slider_(base) {
        juce::ignoreUnused(ui_base_);

        slider_.setSliderStyle(juce::Slider::LinearHorizontal);
        slider_look_and_feel_.setTextAlpha(0.f);
        slider_.setTextBoxIsEditable(false);
        slider_.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        slider_.setDoubleClickReturnValue(true, 0.0);
        slider_.setScrollWheelEnabled(true);
        slider_.setInterceptsMouseClicks(false, false);
        slider_.setLookAndFeel(&slider_look_and_feel_);
        slider_.addListener(this);
        addAndMakeVisible(slider_);

        text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
        text_.setJustificationType(juce::Justification::centred);
        text_look_and_feel_.setAlpha(0.f);
        text_look_and_feel_.setFontScale(kFontHuge);
        text_.setLookAndFeel(&text_look_and_feel_);
        text_.setInterceptsMouseClicks(false, false);
        text_.addListener(this);
        addAndMakeVisible(text_);

        // setup label
        label_.setText(label_text, juce::dontSendNotification);
        label_.setJustificationType(juce::Justification::centred);
        label_.setLookAndFeel(&name_look_and_feel_);
        name_look_and_feel_.setFontScale(kFontHuge);
        label_.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label_);

        setEditable(true);

        if (label_idx != multilingual::Labels::kLabelNum) {
            SettableTooltipClient::setTooltip(ui_base_.getToolTipText(label_idx));
        }
    }

    CompactLinearSlider::~CompactLinearSlider() {
    }

    void CompactLinearSlider::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - lr_pad_,
                                            bound.getHeight() - ub_pad_);
        slider_.setBounds(bound.toNearestInt());
        text_.setBounds(bound.toNearestInt());
        label_.setBounds(bound.toNearestInt());
    }

    void CompactLinearSlider::mouseUp(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
            return;
        }
        slider_.mouseUp(event);
    }

    void CompactLinearSlider::mouseDown(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
            return;
        }
        slider_.mouseDown(event);
    }

    void CompactLinearSlider::mouseDrag(const juce::MouseEvent &event) {
        if (event.mods.isRightButtonDown()) {
            return;
        }
        slider_.mouseDrag(event);
    }

    void CompactLinearSlider::mouseEnter(const juce::MouseEvent &event) {
        text_look_and_feel_.setAlpha(1.f);
        name_look_and_feel_.setAlpha(0.f);
        slider_.mouseEnter(event);
        text_look_and_feel_.setAlpha(1.f);
        name_look_and_feel_.setAlpha(0.f);
        text_.repaint();
        label_.repaint();
    }

    void CompactLinearSlider::mouseExit(const juce::MouseEvent &event) {
        slider_look_and_feel_.setTextAlpha(1.f);
        name_look_and_feel_.setAlpha(0.f);
        slider_.mouseExit(event);

        if (text_.getCurrentTextEditor() != nullptr) {
            return;
        }

        text_look_and_feel_.setAlpha(0.f);
        name_look_and_feel_.setAlpha(1.f);
        text_.repaint();
        label_.repaint();
    }

    void CompactLinearSlider::mouseMove(const juce::MouseEvent &event) {
        slider_.mouseMove(event);
    }

    void CompactLinearSlider::mouseDoubleClick(const juce::MouseEvent &event) {
        if (ui_base_.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
            text_.showEditor();
        } else {
            slider_.mouseDoubleClick(event);
        }
    }

    void CompactLinearSlider::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        slider_.mouseWheelMove(event, wheel);
    }

    void CompactLinearSlider::labelTextChanged(juce::Label *label_that_has_changed) {
        juce::ignoreUnused(label_that_has_changed);
    }

    void CompactLinearSlider::editorShown(juce::Label *l, juce::TextEditor &editor) {
        editor.setInterceptsMouseClicks(false, false);
        juce::ignoreUnused(l);
        editor.setInputRestrictions(0, "-0123456789.kK");
        text_.addMouseListener(this, true);

        editor.setJustification(juce::Justification::centred);
        editor.setColour(juce::TextEditor::outlineColourId, ui_base_.getTextColor());
        editor.setColour(juce::TextEditor::highlightedTextColourId, ui_base_.getTextColor());
        editor.applyFontToAllText(juce::FontOptions{ui_base_.getFontSize() * kFontHuge});
        editor.applyColourToAllText(ui_base_.getTextColor(), true);
    }

    void CompactLinearSlider::editorHidden(juce::Label *l, juce::TextEditor &editor) {
        juce::ignoreUnused(l);
        text_.removeMouseListener(this);
        auto k = 1.0;
        const auto ctext = editor.getText();
        if (ctext.contains("k") || ctext.contains("K")) {
            k = 1000.0;
        }
        const auto actual_value = ctext.getDoubleValue() * k;

        slider_.setValue(actual_value, juce::sendNotificationAsync);

        text_look_and_feel_.setAlpha(0.f);
        name_look_and_feel_.setAlpha(1.f);
        text_.repaint();
        label_.repaint();
    }

    void CompactLinearSlider::sliderValueChanged(juce::Slider *) {
        text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
        text_.repaint();
    }

    juce::String CompactLinearSlider::getDisplayValue(juce::Slider &s) {
        auto value = s.getValue();
        juce::String label_to_display = juce::String(s.getTextFromValue(value)).substring(0, 4);
        if (value < 10000 && label_to_display.contains(".")) {
            label_to_display = juce::String(value).substring(0, 5);
        }
        if (value > 10000) {
            value = value / 1000;
            label_to_display = juce::String(value).substring(0, 4) + "K";
        }
        // remove trailing zeros
        while (label_to_display.contains(".")) {
            const auto last_s = label_to_display.getLastCharacter();
            if (last_s == '.' || last_s == '0') {
                label_to_display = label_to_display.dropLastCharacters(1);
            } else {
                break;
            }
        }
        return label_to_display;
    }
}
