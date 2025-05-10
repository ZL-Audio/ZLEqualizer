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
#include "../../label/name_look_and_feel.hpp"
#include "../extra_slider/snapping_slider.h"

namespace zlgui {
    template<bool Opaque = true, bool UseSecondSlider = true>
    class TwoValueRotarySlider final : public juce::Component,
                                       private juce::Label::Listener,
                                       private juce::Slider::Listener,
                                       public juce::SettableTooltipClient {
    public:
        static constexpr float startAngle = 2.0943951023931953f, endAngle = 7.3303828583761845f;
        std::function<double(juce::String s)> parseString;
        juce::String allowedChars = "-0123456789.kK";

    private:
        class Background final : public juce::Component {
        public:
            explicit Background(UIBase &base) : base_(base) {
                setOpaque(Opaque);
                setInterceptsMouseClicks(false, false);
                setBufferedToImage(true);
            }

            void paint(juce::Graphics &g) override {
                if (Opaque) {
                    g.fillAll(base_.getBackgroundColor());
                }
                auto bounds = getLocalBounds().toFloat();
                const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
                bounds = bounds.withSizeKeepingCentre(diameter, diameter);
                // draw knob background
                const auto oldBounds = base_.drawInnerShadowEllipse(g, bounds, base_.getFontSize() * 0.5f, {});
                const auto newBounds = base_.drawShadowEllipse(g, oldBounds, base_.getFontSize() * 0.5f, {});
                base_.drawInnerShadowEllipse(g, newBounds, base_.getFontSize() * 0.15f, {.flip = true});
            }

        private:
            UIBase &base_;
        };

        class Display final : public juce::Component {
        public:
            explicit Display(UIBase &base) : base_(base) {
                setInterceptsMouseClicks(false, false);
            }

            void paint(juce::Graphics &g) override {
                g.saveState();
                g.reduceClipRegion(mask);

                base_.drawShadowEllipse(g, arrow1, base_.getFontSize() * 0.5f,
                                      {.fit = false, .draw_bright = false, .draw_dark = true});
                g.setColour(base_.getTextHideColor());
                g.fillPath(filling1);
                // fill the pie segment between two values
                if (showSlider2 && UseSecondSlider) {
                    if (value1 > value2) {
                        g.setColour(base_.getColorMap2(0).withAlpha(.75f));
                    } else {
                        g.setColour(base_.getColorMap2(2).withAlpha(.75f));
                    }
                    g.fillPath(filling2);
                }
                g.restoreState();
            }

            void resized() override {
                bound = getLocalBounds().toFloat();
                const auto diameter = juce::jmin(bound.getWidth(), bound.getHeight());
                bound = bound.withSizeKeepingCentre(diameter, diameter);
                oldBound = base_.getInnerShadowEllipseArea(bound, base_.getFontSize() * 0.5f, {});
                newBound = base_.getShadowEllipseArea(oldBound, base_.getFontSize() * 0.5f, {});
                arrowUnit = (diameter - newBound.getWidth()) * 0.5f;

                mask.clear();
                mask.addEllipse(bound);
                mask.setUsingNonZeroWinding(false);
                mask.addEllipse(newBound);

                setSlider1Value(value1);
                if (UseSecondSlider) {
                    setSlider2Value(value2);
                }
            }

            void setSlider1Value(const float x) {
                value1 = x;
                const auto rotationAngle = startAngle + x * (endAngle - startAngle);
                angle1 = rotationAngle;
                filling1.clear();
                filling1.addPieSegment(bound,
                    startAngle + juce::MathConstants<float>::pi * .5f,
                    rotationAngle + juce::MathConstants<float>::pi * .5f,
                    0);
                if (showSlider2 && UseSecondSlider) {
                    setSlider2Value(value2);
                } else {
                    repaint();
                }
            }

            void setSlider2Value(const float x) {
                if (UseSecondSlider) {
                    value2 = x;
                    const auto rotationAngle = startAngle + x * (endAngle - startAngle);
                    filling2.clear();
                    filling2.addPieSegment(bound,
                        angle1 + juce::MathConstants<float>::pi * .5f,
                        rotationAngle + juce::MathConstants<float>::pi * .5f,
                        0);
                    repaint();
                }
            }

            void setShowSlider2(const bool x) {
                if (UseSecondSlider) {
                    showSlider2 = x;
                    repaint();
                }
            }

        private:
            UIBase &base_;
            juce::Rectangle<float> bound, oldBound, newBound;
            float value1{0.f}, value2{0.f};
            float arrowUnit{0.f}, angle1{0.f};
            bool showSlider2{false};
            juce::Path mask;
            juce::Rectangle<float> arrow1, arrow2;
            juce::Path filling1, filling2;
        };

    public:
        explicit TwoValueRotarySlider(const juce::String &label_text, UIBase &base,
                                      multilingual::Labels label_idx = multilingual::Labels::kLabelNum)
            : ui_base_(base), background_(ui_base_),
              slider1_(base, label_text), slider2_(base, label_text), display_(base),
              label_look_and_feel_(base), label_look_and_feel1_(base), label_look_and_feel2_(base), text_box_laf_(base) {
            addAndMakeVisible(background_);
            for (auto const s: {&slider1_, &slider2_}) {
                s->setSliderStyle(ui_base_.getRotaryStyle());
                s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
                s->setDoubleClickReturnValue(true, 0.0);
                s->setScrollWheelEnabled(true);
                s->setInterceptsMouseClicks(false, false);
            }

            slider1_.addListener(this);
            if (UseSecondSlider) {
                slider2_.addListener(this);
            }
            addAndMakeVisible(display_);

            label_.setText(label_text, juce::dontSendNotification);
            label_.setJustificationType(juce::Justification::centred);
            label_.setBufferedToImage(true);
            label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);

            label_look_and_feel_.setFontScale(1.75f);
            label_look_and_feel1_.setFontScale(kFontHuge);
            label1_.setJustificationType(juce::Justification::centredBottom);

            label_.setLookAndFeel(&label_look_and_feel_);
            label1_.setLookAndFeel(&label_look_and_feel1_);

            if (UseSecondSlider) {
                label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
                label_look_and_feel2_.setFontScale(kFontHuge);
                label2_.setJustificationType(juce::Justification::centredTop);
                label2_.setLookAndFeel(&label_look_and_feel2_);
            }

            for (auto &l: {&label_, &label1_, &label2_}) {
                l->setInterceptsMouseClicks(false, false);
            }

            addAndMakeVisible(label_);
            addChildComponent(label1_);

            setEditable(true);
            label1_.setEditable(false, true);

            label1_.addListener(this);

            if (UseSecondSlider) {
                addChildComponent(label2_);
                label2_.setEditable(false, true);
                label2_.addListener(this);
            }

            if (label_idx != multilingual::Labels::kLabelNum) {
                SettableTooltipClient::setTooltip(ui_base_.getToolTipText(label_idx));
            }

            setOpaque(Opaque);
        }

        ~TwoValueRotarySlider() override {
            slider1_.removeListener(this);
            label1_.removeListener(this);
            if (UseSecondSlider) {
                slider2_.removeListener(this);
                label2_.removeListener(this);
            }
        }

        void paint(juce::Graphics &g) override {
            if (Opaque) {
                g.fillAll(ui_base_.getBackgroundColor());
            }
        }

        void mouseUp(const juce::MouseEvent &event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (!show_slider2_ || event.mods.isLeftButtonDown()) {
                slider1_.mouseUp(event);
            } else {
                slider2_.mouseUp(event);
            }
        }

        void mouseDown(const juce::MouseEvent &event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (!show_slider2_ || event.mods.isLeftButtonDown()) {
                slider1_.mouseDown(event);
            } else {
                slider2_.mouseDown(event);
            }
            const auto currentShiftPressed = event.mods.isShiftDown();
            if (currentShiftPressed != is_shift_pressed_) {
                is_shift_pressed_ = currentShiftPressed;
                updateDragDistance();
            }
        }

        void mouseDrag(const juce::MouseEvent &event) override {
            if (!show_slider2_ || event.mods.isLeftButtonDown()) {
                slider1_.mouseDrag(event);
            } else {
                slider2_.mouseDrag(event);
            }
        }

        void mouseEnter(const juce::MouseEvent &event) override {
            slider1_.mouseEnter(event);
            slider2_.mouseEnter(event);
            label_.setVisible(false);
            label1_.setVisible(true);
            label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);
            if (show_slider2_) {
                label2_.setVisible(true);
                label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
            }
        }

        void mouseExit(const juce::MouseEvent &event) override {
            slider1_.mouseExit(event);
            slider2_.mouseExit(event);

            if (label1_.getCurrentTextEditor() != nullptr || label2_.getCurrentTextEditor() != nullptr) {
                return;
            }

            label_.setVisible(true);
            label1_.setVisible(false);
            if (show_slider2_) {
                label2_.setVisible(false);
            }
        }

        void mouseMove(const juce::MouseEvent &event) override {
            juce::ignoreUnused(event);
        }

        void mouseDoubleClick(const juce::MouseEvent &event) override {
            if (ui_base_.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
                const auto portion = static_cast<float>(event.getPosition().getY()
                                     ) / static_cast<float>(getLocalBounds().getHeight());
                if (portion < .5f || !show_slider2_) {
                    label1_.showEditor();
                    return;
                } else {
                    label2_.showEditor();
                    return;
                }
            }
            if (!show_slider2_ || event.mods.isLeftButtonDown()) {
                slider1_.mouseDoubleClick(event);
            } else {
                slider2_.mouseDoubleClick(event);
            }
        }

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override {
            if (!show_slider2_) {
                slider1_.mouseWheelMove(event, wheel);
            } else {
                slider1_.mouseWheelMove(event, wheel);
                slider2_.mouseWheelMove(event, wheel);
            }
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() - lr_pad_,
                                                bound.getHeight() - ub_pad_);
            background_.setBounds(bound.toNearestInt());
            slider1_.setBounds(bound.toNearestInt());
            slider2_.setBounds(bound.toNearestInt());
            display_.setBounds(bound.toNearestInt());

            auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.7f,
                                                          bound.getHeight() * 0.6f);
            label_.setBounds(labelBound.toNearestInt());

            setShowSlider2(show_slider2_);
        }

        inline juce::Slider &getSlider1() { return slider1_; }

        inline juce::Slider &getSlider2() { return slider2_; }

        void setShowSlider2(const bool x) {
            show_slider2_ = x && UseSecondSlider;

            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() - lr_pad_,
                                                bound.getHeight() - ub_pad_);

            auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.6f,
                                                          bound.getHeight() * 0.5f);
            if (show_slider2_) {
                const auto valueBound1 = labelBound.removeFromTop(labelBound.getHeight() * 0.5f);
                const auto valueBound2 = labelBound;
                label1_.setBounds(valueBound1.toNearestInt());
                label2_.setBounds(valueBound2.toNearestInt());
                label1_.setJustificationType(juce::Justification::centredBottom);
                label2_.setJustificationType(juce::Justification::centredTop);
            } else {
                labelBound = labelBound.withSizeKeepingCentre(labelBound.getWidth(), labelBound.getHeight() * .5f);
                label1_.setBounds(labelBound.toNearestInt());
                label2_.setVisible(false);
                label1_.setJustificationType(juce::Justification::centred);
            }
            display_.setShowSlider2(show_slider2_);
        }

        inline void setPadding(const float lr, const float ub) {
            lr_pad_ = lr;
            ub_pad_ = ub;
        }

        inline void setEditable(const bool x) {
            editable_ = x;
            label_look_and_feel_.setEditable(x);
            label_look_and_feel1_.setEditable(x);
            label_look_and_feel2_.setEditable(x);
            setInterceptsMouseClicks(x, false);
            label_.repaint();
        }

        inline bool getEditable() const { return editable_; }

        void setMouseDragSensitivity(const int x) {
            drag_distance_ = x;
            updateDragDistance();
        }

        void updateDisplay() {
            display_.setSlider1Value(
                    static_cast<float>(slider1_.getNormalisableRange().convertTo0to1(slider1_.getValue())));
            display_.setSlider2Value(
                    static_cast<float>(slider2_.getNormalisableRange().convertTo0to1(slider2_.getValue())));
        }

    private:
        UIBase &ui_base_;
        Background background_;

        SnappingSlider slider1_, slider2_;

        Display display_;

        NameLookAndFeel label_look_and_feel_, label_look_and_feel1_, label_look_and_feel2_;
        NameLookAndFeel text_box_laf_;

        juce::Label label_, label1_, label2_;

        bool show_slider2_{false}, editable_{true};
        float lr_pad_{0.f}, ub_pad_{0.f};

        int drag_distance_{10};
        bool is_shift_pressed_{false};

        static juce::String getDisplayValue(juce::Slider &s) {
            auto value = s.getNormalisableRange().snapToLegalValue(s.getValue());
            juce::String label_to_display = juce::String(value).substring(0, 4);
            if (value < 10000 && label_to_display.contains(".")) {
                label_to_display = juce::String(value).substring(0, 5);
            }
            if (value >= 10000) {
                value = value / 1000;
                label_to_display = juce::String(value).substring(0, 4) + "K";
            }
            // remove trailing zeros
            while (label_to_display.contains(".")) {
                const auto lastS = label_to_display.getLastCharacter();
                if (lastS == '.' || lastS == '0') {
                    label_to_display = label_to_display.dropLastCharacters(1);
                } else {
                    break;
                }
            }
            return label_to_display;
        }

        void labelTextChanged(juce::Label *label_that_has_changed) override {
            juce::ignoreUnused(label_that_has_changed);
        }

        void editorShown(juce::Label *l, juce::TextEditor &editor) override {
            juce::ignoreUnused(l);
            editor.setInputRestrictions(0, allowedChars);

            label_.setVisible(false);
            label1_.setVisible(true);
            if (show_slider2_) {
                label2_.setVisible(true);
            }

            editor.setJustification(juce::Justification::centred);
            editor.setColour(juce::TextEditor::outlineColourId, ui_base_.getTextColor());
            editor.setColour(juce::TextEditor::highlightedTextColourId, ui_base_.getTextColor());
            editor.applyFontToAllText(juce::FontOptions{ui_base_.getFontSize() * kFontHuge});
            editor.applyColourToAllText(ui_base_.getTextColor(), true);
        }

        void editorHidden(juce::Label *l, juce::TextEditor &editor) override {
            double actual_value;
            if (parseString) {
                actual_value = parseString(editor.getText());
            } else {
                const auto text = editor.getText();
                const auto k = (text.contains("k") || text.contains("K")) ? 1000.0 : 1.0;
                actual_value = text.getDoubleValue() * k;
            }

            if (l == &label1_) {
                slider1_.setValue(actual_value, juce::sendNotificationAsync);
            }
            if (l == &label2_) {
                slider2_.setValue(actual_value, juce::sendNotificationAsync);
            }

            label_.setVisible(true);
            label1_.setVisible(false);
            if (show_slider2_) {
                label2_.setVisible(false);
            }
        }

        void sliderValueChanged(juce::Slider *slider) override {
            if (slider == &slider1_) {
                label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);
                display_.setSlider1Value(
                    static_cast<float>(slider1_.getNormalisableRange().convertTo0to1(slider1_.getValue())));
            }
            if (slider == &slider2_) {
                label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
                display_.setSlider2Value(
                    static_cast<float>(slider2_.getNormalisableRange().convertTo0to1(slider2_.getValue())));
            }
        }

        void updateDragDistance() {
            int actual_drag_distance;
            if (is_shift_pressed_) {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) / ui_base_.getSensitivity(SensitivityIdx::kMouseDragFine));
            } else {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) / ui_base_.getSensitivity(SensitivityIdx::kMouseDrag));
            }
            actual_drag_distance = std::max(actual_drag_distance, 1);
            slider1_.setMouseDragSensitivity(actual_drag_distance);
            slider2_.setMouseDragSensitivity(actual_drag_distance);
        }
    };
}
