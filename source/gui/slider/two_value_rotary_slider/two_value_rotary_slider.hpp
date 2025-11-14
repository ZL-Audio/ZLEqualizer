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

namespace zlgui::slider {
    template <bool kOpaque = true, bool kUseSecondSlider = true, bool kUseName = true>
    class TwoValueRotarySlider final : public juce::Component,
                                       private juce::Label::Listener,
                                       private juce::Slider::Listener,
                                       public juce::SettableTooltipClient {
    public:
        static constexpr float kStartAngle = 2.0943951023931953f, kEndAngle = 7.3303828583761845f;
        std::string permitted_characters_ = "-0123456789.kK";
        std::function<std::string(double)> value_formatter_;
        std::function<std::optional<double>(const std::string&)> string_formatter_;

    private:
        class Background final : public juce::Component {
        public:
            explicit Background(UIBase& base, const float thick_scale) :
                base_(base), thick_scale_(thick_scale) {
                setOpaque(kOpaque);
                setInterceptsMouseClicks(false, false);
                setBufferedToImage(true);
            }

            void paint(juce::Graphics& g) override {
                if constexpr (kOpaque) {
                    g.fillAll(base_.getBackgroundColour());
                } else {
                    g.setColour(base_.getBackgroundColour());
                    g.fillEllipse(getLocalBounds().toFloat());
                }
                auto bounds = getLocalBounds().toFloat();
                const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
                bounds = bounds.withSizeKeepingCentre(diameter, diameter);
                juce::Path mask;
                mask.addPieSegment(bounds, kStartAngle - juce::MathConstants<float>::pi * 1.5f,
                                     kEndAngle - juce::MathConstants<float>::pi * 1.5f,
                                     0);
                g.saveState();
                g.reduceClipRegion(mask);
                // draw knob background
                const auto old_bounds = base_.drawInnerShadowEllipse(
                    g, bounds, base_.getFontSize() * 0.5f * thick_scale_, {});
                const auto new_bounds = base_.drawShadowEllipse(g, old_bounds,
                                                                base_.getFontSize() * 0.5f * thick_scale_, {});
                base_.drawInnerShadowEllipse(g, new_bounds, base_.getFontSize() * 0.15f, {.flip = true});
                g.restoreState();
            }

        private:
            UIBase& base_;
            float thick_scale_{1.f};
        };

        class Display final : public juce::Component {
        public:
            explicit Display(UIBase& base, const float thick_scale) :
                base_(base), thick_scale_(thick_scale) {
                setInterceptsMouseClicks(false, false);
            }

            void paint(juce::Graphics& g) override {
                g.saveState();
                g.reduceClipRegion(mask_);
                g.setColour(base_.getTextHideColour());
                g.fillPath(filling1_);
                // fill the pie segment between two values
                if constexpr (kUseSecondSlider) {
                    if (show_slider2_) {
                        if (value1_ > value2_) {
                            g.setColour(base_.getColourMap2(0).withAlpha(.75f));
                        } else {
                            g.setColour(base_.getColourMap2(2).withAlpha(.75f));
                        }
                        g.fillPath(filling2_);
                    }
                }
                g.restoreState();
            }

            void resized() override {
                bound_ = getLocalBounds().toFloat();
                const auto diameter = juce::jmin(bound_.getWidth(), bound_.getHeight());
                bound_ = bound_.withSizeKeepingCentre(diameter, diameter);
                old_bound_ = UIBase::getInnerShadowEllipseArea(bound_, base_.getFontSize() * 0.5f * thick_scale_, {});
                new_bound_ = UIBase::getShadowEllipseArea(old_bound_, base_.getFontSize() * 0.5f * thick_scale_, {});
                arrow_unit_ = (diameter - new_bound_.getWidth()) * 0.5f;

                mask_.clear();
                mask_.addEllipse(bound_);
                mask_.setUsingNonZeroWinding(false);
                mask_.addEllipse(new_bound_);

                setSlider1Value(value1_);
                if constexpr (kUseSecondSlider) {
                    setSlider2Value(value2_);
                }
            }

            void setSlider1Value(const float x) {
                value1_ = x;
                const auto rotationAngle = kStartAngle + x * (kEndAngle - kStartAngle);
                angle1_ = rotationAngle;
                filling1_.clear();
                filling1_.addPieSegment(bound_,
                                        kStartAngle + juce::MathConstants<float>::pi * .5f,
                                        rotationAngle + juce::MathConstants<float>::pi * .5f,
                                        0);
                if (kUseSecondSlider && show_slider2_) {
                    setSlider2Value(value2_);
                } else {
                    repaint();
                }
            }

            void setSlider2Value(const float x) {
                if constexpr (kUseSecondSlider) {
                    value2_ = x;
                    const auto rotationAngle = kStartAngle + x * (kEndAngle - kStartAngle);
                    filling2_.clear();
                    filling2_.addPieSegment(bound_,
                                            angle1_ + juce::MathConstants<float>::pi * .5f,
                                            rotationAngle + juce::MathConstants<float>::pi * .5f,
                                            0);
                    repaint();
                }
            }

            void setShowSlider2(const bool x) {
                if constexpr (kUseSecondSlider) {
                    show_slider2_ = x;
                }
            }

        private:
            UIBase& base_;
            float thick_scale_{1.f};
            juce::Rectangle<float> bound_, old_bound_, new_bound_;
            float value1_{0.f}, value2_{0.f};
            float arrow_unit_{0.f}, angle1_{0.f};
            bool show_slider2_{false};
            juce::Path mask_;
            juce::Rectangle<float> arrow1_, arrow2_;
            juce::Path filling1_, filling2_;
        };

    public:
        explicit TwoValueRotarySlider(const juce::String& label_text, UIBase& base,
                                      const juce::String& tooltip_text = "",
                                      float thick_scale = 1.f) :
            base_(base), background_(base_, thick_scale), display_(base, thick_scale),
            slider1_(base, label_text), slider2_(base, label_text),
            label_look_and_feel_(base), label_look_and_feel1_(base), label_look_and_feel2_(base),
            text_box_laf_(base) {
            addAndMakeVisible(background_);
            for (auto const s : {&slider1_, &slider2_}) {
                s->setSliderStyle(base_.getRotaryStyle());
                s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
                s->setDoubleClickReturnValue(true, 0.0);
                s->setScrollWheelEnabled(true);
                s->setInterceptsMouseClicks(false, false);
            }

            slider1_.addListener(this);
            if constexpr (kUseSecondSlider) {
                slider2_.addListener(this);
            }
            addAndMakeVisible(display_);

            if constexpr (kUseName) {
                label_.setText(label_text, juce::dontSendNotification);
                label_.setJustificationType(juce::Justification::centred);
                label_.setBufferedToImage(true);
                label_look_and_feel_.setFontScale(1.75f);
                label_.setLookAndFeel(&label_look_and_feel_);
            }

            label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);
            label_look_and_feel1_.setFontScale(1.5f);
            label1_.setJustificationType(juce::Justification::centredBottom);
            label1_.setLookAndFeel(&label_look_and_feel1_);

            if constexpr (kUseSecondSlider) {
                label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
                label2_.setJustificationType(juce::Justification::centredTop);
                label2_.setLookAndFeel(&label_look_and_feel1_);
            }

            for (auto& l : {&label_, &label1_, &label2_}) {
                l->setInterceptsMouseClicks(false, false);
            }

            if constexpr (kUseName) {
                addAndMakeVisible(label_);
                addChildComponent(label1_);
            } else {
                addAndMakeVisible(label1_);
            }

            setEditable(true);
            label1_.setEditable(false, true);
            label1_.addListener(this);

            if constexpr (kUseSecondSlider) {
                addChildComponent(label2_);
                label2_.setEditable(false, true);
                label2_.addListener(this);
            }

            // set up tooltip
            if (tooltip_text.length() > 0) {
                SettableTooltipClient::setTooltip(tooltip_text);
            }

            setOpaque(kOpaque);
        }

        ~TwoValueRotarySlider() override {
            slider1_.removeListener(this);
            label1_.removeListener(this);
            if constexpr (kUseSecondSlider) {
                slider2_.removeListener(this);
                label2_.removeListener(this);
            }
        }

        void visibilityChanged() override {
            if (isVisible()) {
                sliderValueChanged(&slider1_);
                if constexpr (kUseSecondSlider) {
                    sliderValueChanged(&slider2_);
                }
            }
        }

        void paint(juce::Graphics& g) override {
            juce::ignoreUnused(g);
        }

        void mouseUp(const juce::MouseEvent& event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (event.mods.isLeftButtonDown()) {
                slider1_.mouseUp(event);
            } else if (show_slider2_ && event.mods.isRightButtonDown()) {
                slider2_.mouseUp(event);
            }
        }

        void mouseDown(const juce::MouseEvent& event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (event.mods.isLeftButtonDown()) {
                slider1_.mouseDown(event);
            } else if (show_slider2_ && event.mods.isRightButtonDown()) {
                slider2_.mouseDown(event);
            }
            const auto currentShiftPressed = event.mods.isShiftDown();
            if (currentShiftPressed != is_shift_pressed_) {
                is_shift_pressed_ = currentShiftPressed;
                updateDragDistance();
            }
        }

        void mouseDrag(const juce::MouseEvent& event) override {
            if (event.mods.isLeftButtonDown()) {
                slider1_.mouseDrag(event);
            } else if (show_slider2_ && event.mods.isRightButtonDown()) {
                slider2_.mouseDrag(event);
            }
        }

        void mouseEnter(const juce::MouseEvent& event) override {
            slider1_.mouseEnter(event);
            slider2_.mouseEnter(event);
            if constexpr (kUseName) {
                label_.setVisible(false);
                label1_.setVisible(true);
                label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);
                if constexpr (kUseSecondSlider) {
                    if (show_slider2_) {
                        label2_.setVisible(true);
                        label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
                    }
                }
            }
        }

        void mouseExit(const juce::MouseEvent& event) override {
            slider1_.mouseExit(event);
            slider2_.mouseExit(event);
            if constexpr (kUseName) {
                if (label1_.getCurrentTextEditor() != nullptr || label2_.getCurrentTextEditor() != nullptr) {
                    return;
                }

                label_.setVisible(true);
                label1_.setVisible(false);
                if constexpr (kUseSecondSlider) {
                    if (show_slider2_) {
                        label2_.setVisible(false);
                    }
                }
            }
        }

        void mouseMove(const juce::MouseEvent& event) override {
            juce::ignoreUnused(event);
        }

        void mouseDoubleClick(const juce::MouseEvent& event) override {
            if (base_.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
                const auto portion = event.position.y / static_cast<float>(getLocalBounds().getHeight());
                if (portion < .5f || !show_slider2_) {
                    label1_.showEditor();
                    return;
                } else {
                    label2_.showEditor();
                    return;
                }
            }
            if (event.mods.isLeftButtonDown()) {
                slider1_.mouseDoubleClick(event);
            } else if (show_slider2_ && event.mods.isRightButtonDown()) {
                slider2_.mouseDoubleClick(event);
            }
        }

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override {
            if (!show_slider2_) {
                slider1_.mouseWheelMove(event, wheel);
            } else {
                slider1_.mouseWheelMove(event, wheel);
                slider2_.mouseWheelMove(event, wheel);
            }
        }

        void resized() override {
            const auto bound = getLocalBounds();
            background_.setBounds(bound);
            display_.setBounds(bound);
            slider1_.setBounds(bound);
            if constexpr (kUseSecondSlider) {
                slider2_.setBounds(bound);
            }
            if constexpr (kUseName) {
                const auto bound_size = std::min(bound.getWidth(), bound.getHeight());
                const auto label_bound = bound.withSizeKeepingCentre(
                    static_cast<int>(std::round(static_cast<float>(bound_size * 0.7f))),
                    static_cast<int>(std::round(static_cast<float>(bound_size * 0.6f))));
                label_.setBounds(label_bound);
            }

            setShowSlider2(show_slider2_);
        }

        inline juce::Slider& getSlider1() { return slider1_; }

        inline juce::Slider& getSlider2() { return slider2_; }

        void setShowSlider2(const bool x) {
            show_slider2_ = x && kUseSecondSlider;

            const auto bound = getLocalBounds().toFloat();

            auto label_bound = bound.withSizeKeepingCentre(bound.getWidth() * 0.7f,
                                                           bound.getHeight() * 0.5f);
            if (show_slider2_) {
                const auto valueBound1 = label_bound.removeFromTop(label_bound.getHeight() * 0.5f);
                const auto valueBound2 = label_bound;
                label1_.setBounds(valueBound1.toNearestInt());
                label1_.setJustificationType(juce::Justification::centredBottom);
                label2_.setVisible(true);
                label2_.setBounds(valueBound2.toNearestInt());
                label2_.setJustificationType(juce::Justification::centredTop);
            } else {
                label_bound = label_bound.withSizeKeepingCentre(label_bound.getWidth(), label_bound.getHeight() * .5f);
                label1_.setBounds(label_bound.toNearestInt());
                label2_.setVisible(false);
                label1_.setJustificationType(juce::Justification::centred);
            }
            display_.setShowSlider2(show_slider2_);
            sliderValueChanged(&slider2_);
        }

        inline void setEditable(const bool x) {
            setAlpha(x ? 1.f : .5f);
            setInterceptsMouseClicks(x, false);
        }

        void setMouseDragSensitivity(const int x) {
            drag_distance_ = x;
            updateDragDistance();
        }

        void setPrecision(const int x) {
            precision_ = x;
        }

    private:
        UIBase& base_;
        Background background_;
        Display display_;

        SnappingSlider slider1_, slider2_;

        label::NameLookAndFeel label_look_and_feel_, label_look_and_feel1_, label_look_and_feel2_;
        label::NameLookAndFeel text_box_laf_;

        juce::Label label_, label1_, label2_;

        bool show_slider2_{false};

        int drag_distance_{10};
        bool is_shift_pressed_{false};

        int precision_{4};

        juce::String getDisplayValue(const juce::Slider& s) const {
            const auto value = s.getValue();
            if (value_formatter_) {
                return value_formatter_(value);
            }
            const bool append_k = precision_ >= 4 ? std::abs(value) >= 10000.0 : std::abs(value) >= 1000.0;
            const auto display_value = append_k ? value * 0.001 : value;
            auto actual_precision = append_k ? precision_ - 1 : precision_;
            if (std::abs(display_value) >= 100.0) {
                actual_precision = std::max(actual_precision, 3);
            }

            char buffer[32];
            if (std::abs(value) < 1.0) {
                snprintf(buffer, sizeof(buffer), "%.*f", actual_precision - 1, display_value);
            } else {
                snprintf(buffer, sizeof(buffer), "%.*g", actual_precision, display_value);
            }
            std::string str{buffer};
            // remove trailing zeros and decimal point
            const auto last_decimal = str.find_last_of('.');
            if (last_decimal != std::string::npos) {
                const auto last_not_zero = str.find_last_not_of('0');
                if (last_not_zero != std::string::npos) {
                    str.erase(last_not_zero + 1);
                }
                if (str.back() == '.') {
                    str.pop_back();
                }
            }

            return append_k ? juce::String{str + "K"} : juce::String{str};
        }

        void labelTextChanged(juce::Label* label_that_has_changed) override {
            juce::ignoreUnused(label_that_has_changed);
        }

        void editorShown(juce::Label* l, juce::TextEditor& editor) override {
            juce::ignoreUnused(l);
            editor.setInputRestrictions(0, permitted_characters_);

            if constexpr (kUseName) {
                label_.setVisible(false);
                label1_.setVisible(true);
                if constexpr (kUseSecondSlider) {
                    if (show_slider2_) {
                        label2_.setVisible(true);
                    }
                }
            }

            editor.setJustification(juce::Justification::centred);
            editor.setColour(juce::TextEditor::outlineColourId, base_.getTextColour());
            editor.setColour(juce::TextEditor::highlightedTextColourId, base_.getTextColour());
            editor.applyFontToAllText(juce::FontOptions{base_.getFontSize() * kFontHuge});
            editor.applyColourToAllText(base_.getTextColour(), true);
        }

        void editorHidden(juce::Label* l, juce::TextEditor& editor) override {
            const auto ctext = editor.getText();
            std::optional<double> format_result{std::nullopt};
            if (string_formatter_) {
                format_result = string_formatter_(ctext.toStdString());
            }
            double actual_value;
            if (format_result != std::nullopt) {
                actual_value = format_result.value();
            } else {
                const auto k = ctext.contains("k") || ctext.contains("K") ? 1000.0 : 1.0;
                actual_value = ctext.getDoubleValue() * k;
            }

            if (l == &label1_) {
                slider1_.setValue(actual_value, juce::sendNotificationAsync);
            }
            if constexpr (kUseSecondSlider) {
                if (l == &label2_) {
                    slider2_.setValue(actual_value, juce::sendNotificationAsync);
                }
            }
            if constexpr (kUseName) {
                label_.setVisible(true);
                label1_.setVisible(false);
                if constexpr (kUseSecondSlider) {
                    if (show_slider2_) {
                        label2_.setVisible(false);
                    }
                }
            }
        }

        void sliderValueChanged(juce::Slider* slider) override {
            if (slider == &slider1_) {
                label1_.setText(getDisplayValue(slider1_), juce::dontSendNotification);
                display_.setSlider1Value(
                    static_cast<float>(slider1_.getNormalisableRange().convertTo0to1(slider1_.getValue())));
            }
            if constexpr (kUseSecondSlider) {
                if (slider == &slider2_) {
                    label2_.setText(getDisplayValue(slider2_), juce::dontSendNotification);
                    display_.setSlider2Value(
                        static_cast<float>(slider2_.getNormalisableRange().convertTo0to1(slider2_.getValue())));
                }
            }
        }

        void updateDragDistance() {
            int actual_drag_distance;
            if (is_shift_pressed_) {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) / base_.getSensitivity(SensitivityIdx::kMouseDragFine));
            } else {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) / base_.getSensitivity(SensitivityIdx::kMouseDrag));
            }
            actual_drag_distance = std::max(actual_drag_distance, 1);
            slider1_.setMouseDragSensitivity(actual_drag_distance);
            if constexpr (kUseSecondSlider) {
                slider2_.setMouseDragSensitivity(actual_drag_distance);
            }
        }
    };
}
