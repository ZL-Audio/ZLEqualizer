// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../label/name_look_and_feel.hpp"
#include "../extra_slider/snapping_slider.h"
#include <cstdio>

namespace zlgui::slider {
    template <bool kUseBackground = true, bool kUseDisplay = true, bool kUseName = true>
    class CompactLinearSlider final : public juce::Component,
                                      private juce::Label::Listener,
                                      private juce::Slider::Listener,
                                      public juce::SettableTooltipClient {
    private:
        class Background final : public juce::Component {
        public:
            explicit Background(UIBase& base) : base_(base) {
                setInterceptsMouseClicks(false, false);
                setBufferedToImage(true);
            }

            void paint(juce::Graphics& g) override {
                base_.fillRoundedInnerShadowRectangle(g, getLocalBounds().toFloat(),
                                                      base_.getFontSize() * 0.5f, {.blur_radius = 0.66f});
            }

        private:
            UIBase& base_;
        };

        class Display final : public juce::Component {
        public:
            explicit Display(UIBase& base) : base_(base) {
                setInterceptsMouseClicks(false, false);
            }

            void paint(juce::Graphics& g) override {
                auto bound = getLocalBounds().toFloat();
                bound = bound.withWidth(value_ * bound.getWidth());
                g.saveState();
                g.reduceClipRegion(mask_);
                g.setColour(base_.getTextHideColor());
                g.fillRect(bound);
                g.restoreState();
            }

            void resized() override {
                setSliderValue(value_);
            }

            void setSliderValue(const float x) {
                value_ = x;
                mask_.clear();
                mask_.addRoundedRectangle(getLocalBounds().toFloat(), base_.getFontSize() * 0.5f);
                repaint();
            }

        private:
            UIBase& base_;
            float value_{0.f};
            juce::Path mask_;
        };

    public:
        explicit CompactLinearSlider(const juce::String& label_text, UIBase& base,
                                     const juce::String& tooltip_text = "")
            : base_(base), background_(base_), display_(base_),
              slider_(base_),
              name_look_and_feel_(base_), text_look_and_feel_(base_) {
            juce::ignoreUnused(base_);

            slider_.setSliderStyle(juce::Slider::LinearHorizontal);
            slider_.setTextBoxIsEditable(false);
            slider_.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            slider_.setDoubleClickReturnValue(true, 0.0);
            slider_.setScrollWheelEnabled(true);
            slider_.setInterceptsMouseClicks(false, false);
            slider_.addListener(this);

            if constexpr (kUseBackground) {
                addAndMakeVisible(background_);
            }
            if constexpr (kUseDisplay) {
                addAndMakeVisible(display_);
            }

            text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
            text_.setJustificationType(juce::Justification::centred);
            text_look_and_feel_.setFontScale(font_scale_);
            text_.setLookAndFeel(&text_look_and_feel_);
            text_.setInterceptsMouseClicks(false, false);
            text_.addListener(this);
            addAndMakeVisible(text_);

            // setup label
            if constexpr (kUseName) {
                text_look_and_feel_.setAlpha(0.f);
                label_.setText(label_text, juce::dontSendNotification);
                label_.setJustificationType(juce::Justification::centred);
                label_.setLookAndFeel(&name_look_and_feel_);
                name_look_and_feel_.setFontScale(font_scale_);
                label_.setInterceptsMouseClicks(false, false);
                addAndMakeVisible(label_);
            }

            // set up tooltip
            if (tooltip_text.length() > 0) {
                SettableTooltipClient::setTooltip(tooltip_text);
            }

            setEditable(true);
        }

        ~CompactLinearSlider() override = default;

        void visibilityChanged() override {
            if (isVisible()) {
                sliderValueChanged(&slider_);
            }
        }

        void resized() override {
            const auto bound = getLocalBounds();
            if constexpr (kUseBackground) {
                background_.setBounds(bound);
            }
            if constexpr (kUseDisplay) {
                display_.setBounds(bound);
            }
            slider_.setBounds(bound);
            text_.setBounds(bound);
            if constexpr (kUseName) {
                label_.setBounds(bound);
            }
        }

        void mouseUp(const juce::MouseEvent& event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
                return;
            }
            slider_.mouseUp(event);
        }

        void mouseDown(const juce::MouseEvent& event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
                return;
            }
            slider_.mouseDown(event);
        }

        void mouseDrag(const juce::MouseEvent& event) override {
            if (event.mods.isRightButtonDown()) {
                return;
            }
            slider_.mouseDrag(event);
        }

        void mouseEnter(const juce::MouseEvent& event) override {
            slider_.mouseEnter(event);
            if constexpr (kUseName) {
                text_look_and_feel_.setAlpha(1.f);
                name_look_and_feel_.setAlpha(0.f);
                text_.repaint();
                label_.repaint();
            }
        }

        void mouseExit(const juce::MouseEvent& event) override {
            slider_.mouseExit(event);
            if (text_.getCurrentTextEditor() != nullptr) {
                return;
            }
            if constexpr (kUseName) {
                text_look_and_feel_.setAlpha(0.f);
                name_look_and_feel_.setAlpha(1.f);
                text_.repaint();
                label_.repaint();
            }
        }

        void mouseMove(const juce::MouseEvent& event) override {
            slider_.mouseMove(event);
        }

        void mouseDoubleClick(const juce::MouseEvent& event) override {
            if (base_.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
                text_.showEditor();
            } else {
                slider_.mouseDoubleClick(event);
            }
        }

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override {
            slider_.mouseWheelMove(event, wheel);
        }

        inline juce::Slider& getSlider() { return slider_; }

        inline void setEditable(const bool x) {
            setAlpha(x ? 1.f : .5f);
            setInterceptsMouseClicks(x, false);
        }

        void updateDisplayValue() {
            text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
            text_.repaint();
        }

        void setFontScale(const float scale) {
            font_scale_ = scale;
            text_look_and_feel_.setFontScale(font_scale_);
            name_look_and_feel_.setFontScale(font_scale_);
        }

        void setPrecision(const int x) {
            precision_ = std::max(x, 2);
        }

        void setJustification(const juce::Justification justification) {
            label_.setJustificationType(justification);
            text_.setJustificationType(justification);
        }

    private:
        UIBase& base_;
        Background background_;
        Display display_;

        SnappingSlider slider_;

        label::NameLookAndFeel name_look_and_feel_, text_look_and_feel_;
        juce::Label label_, text_;

        float font_scale_{1.5f};

        int precision_{4};

        juce::String getDisplayValue(const juce::Slider& s) const {
            const auto value = s.getValue();
            const bool append_k = precision_ >= 4 ? std::abs(value) >= 10000.0 : std::abs(value) >= 1000.0;
            const auto display_value = append_k ? value * 0.001f : value;
            const auto actual_precision = append_k ? precision_ - 1 : precision_;

            char buffer[32];
            if (std::abs(value) < 1.0) {
                snprintf(buffer, sizeof(buffer), "%.*f", actual_precision - 1, display_value);
            } else {
                snprintf(buffer, sizeof(buffer), "%.*g", actual_precision, display_value);
            }
            const std::string str{buffer};

            return append_k ? juce::String{str + "K"} : juce::String{str};
        }

        void labelTextChanged(juce::Label*) override {
        }

        void editorShown(juce::Label*, juce::TextEditor& editor) override {
            editor.setInterceptsMouseClicks(false, false);
            editor.setInputRestrictions(0, "-0123456789.kK");
            text_.addMouseListener(this, true);

            editor.setJustification(juce::Justification::centred);
            editor.setColour(juce::TextEditor::outlineColourId, base_.getTextColor());
            editor.setColour(juce::TextEditor::highlightedTextColourId, base_.getTextColor());
            editor.applyFontToAllText(juce::FontOptions{base_.getFontSize() * font_scale_});
            editor.applyColourToAllText(base_.getTextColor(), true);
        }

        void editorHidden(juce::Label*, juce::TextEditor& editor) override {
            text_.removeMouseListener(this);
            auto k = 1.0;
            const auto ctext = editor.getText();
            if (ctext.contains("k") || ctext.contains("K")) {
                k = 1000.0;
            }
            const auto actual_value = ctext.getDoubleValue() * k;

            slider_.setValue(actual_value, juce::sendNotificationAsync);
            if constexpr (kUseName) {
                text_look_and_feel_.setAlpha(0.f);
                name_look_and_feel_.setAlpha(1.f);
                text_.repaint();
                label_.repaint();
            }
        }

        void sliderValueChanged(juce::Slider*) override {
            text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
            display_.setSliderValue(
                static_cast<float>(slider_.getNormalisableRange().convertTo0to1(slider_.getValue())));
        }
    };
}
