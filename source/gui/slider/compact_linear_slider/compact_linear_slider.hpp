// Copyright (C) 2026 - zsliu98
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

namespace zlgui::slider {
    template <bool kUseBackground = true, bool kUseDisplay = true, bool kUseName = true>
    class CompactLinearSlider final : public juce::Component,
                                      private juce::Label::Listener,
                                      private juce::Slider::Listener,
                                      public juce::SettableTooltipClient {
    public:
        std::string permitted_characters_ = "-0123456789.kK";
        std::function<std::string(double)> value_formatter_;
        std::function<std::optional<double>(const std::string&)> string_formatter_;

    private:
        static constexpr float kFlatTrackAlpha = 0.1f;
        static constexpr float kFlatBarThickness = 0.4f;
        static constexpr float kFlatCornerSize = kFlatBarThickness * .5f;
        static constexpr float kFlatMarkerExtension = 0.15f;
        static constexpr float kFlatMarkerWidth = 0.30f;

        static int getFlatMarkerWidth(const float font_size, const int slider_width) {
            return std::min(juce::roundToInt(font_size * kFlatMarkerWidth), std::max(0, slider_width - 2));
        }

        class Background final : public juce::Component {
        public:
            explicit Background(UIBase& base) :
                base_(base) {
                setInterceptsMouseClicks(false, false);
                updateBuffering();
            }

            void paint(juce::Graphics& g) override {
                const auto bound = getLocalBounds().toFloat();
                if (base_.getUIStyle() == UIStyle::kFlat) {
                    g.setColour(base_.getColourBlendedWithBackground(base_.getTextColour(), kFlatTrackAlpha));
                    g.fillRoundedRectangle(bound, base_.getFontSize() * kFlatCornerSize);
                } else {
                    base_.fillRoundedInnerShadowRectangle(g, bound, base_.getFontSize() * .5f, {.blur_radius = 0.66f});
                }
            }

            void lookAndFeelChanged() override {
                updateBuffering();
            }

        private:
            UIBase& base_;

            void updateBuffering() {
                const auto should_buffer = base_.getUIStyle() != UIStyle::kFlat;
                if (should_buffer != (getCachedComponentImage() != nullptr)) {
                    setBufferedToImage(should_buffer);
                }
            }
        };

        class Display final : public juce::Component {
        public:
            explicit Display(UIBase& base) :
                base_(base) {
                setInterceptsMouseClicks(false, false);
            }

            void paint(juce::Graphics& g) override {
                const auto flat = base_.getUIStyle() == UIStyle::kFlat;
                const auto highlighted = hovered_ && flat;
                const auto text_colour = base_.getTextColour();

                if (fill_mode_ != FillMode::kEmpty) {
                    g.setColour(text_colour.withAlpha(highlighted ? .5f : .25f));
                    switch (fill_mode_) {
                        case FillMode::kFull:
                            g.fillRoundedRectangle(track_bound_, corner_size_);
                            break;
                        case FillMode::kRect:
                            g.fillRect(fill_bound_);
                            break;
                        case FillMode::kPath:
                            g.fillPath(fill_path_);
                            break;
                        case FillMode::kEmpty:
                            break;
                    }
                }

                if (flat && !track_bound_.isEmpty()) {
                    const auto marker = track_bound_.withWidth(marker_width_)
                                            .withX(fill_bound_.getRight() - marker_width_ * .5f);
                    g.setColour(text_colour);
                    g.fillRect(marker.expanded(0.f, marker_extension_));
                }
            }

            void setTrackBounds(const juce::Rectangle<int> bound, const int marker_width, const int marker_extension) {
                track_bound_ = bound.toFloat();
                marker_width_ = static_cast<float>(marker_width);
                marker_extension_ = static_cast<float>(marker_extension);
                const auto corner_scale = base_.getUIStyle() == UIStyle::kFlat ? kFlatCornerSize : .5f;
                corner_size_ = std::max(0.f, base_.getFontSize() * corner_scale);
                corner_x_ = std::min(corner_size_, track_bound_.getWidth() * .5f);
                corner_y_ = std::min(corner_size_, track_bound_.getHeight() * .5f);
                updateFillGeometry();
                repaint();
            }

            void setHovered(const bool hovered) {
                if (hovered_ == hovered) {
                    return;
                }
                hovered_ = hovered;
                if (base_.getUIStyle() == UIStyle::kFlat && !track_bound_.isEmpty()) {
                    repaint(track_bound_.getSmallestIntegerContainer());
                }
            }

            void setSliderValue(const float x) {
                if (std::abs(value_ - x) <= 1e-6f) {
                    return;
                }
                const auto old_right = fill_bound_.getRight();
                value_ = x;
                updateFillGeometry();
                if (track_bound_.isEmpty()) {
                    return;
                }
                const auto new_right = fill_bound_.getRight();
                const auto flat = base_.getUIStyle() == UIStyle::kFlat;
                const auto half_marker = flat ? marker_width_ * .5f : 0.f;
                const auto ext = flat ? marker_extension_ : 0.f;
                const auto left = static_cast<int>(std::floor(std::min(old_right, new_right) - half_marker)) - 1;
                const auto right = static_cast<int>(std::ceil(std::max(old_right, new_right) + half_marker)) + 1;
                const auto top = static_cast<int>(std::floor(track_bound_.getY() - ext)) - 1;
                const auto bottom = static_cast<int>(std::ceil(track_bound_.getBottom() + ext)) + 1;
                const auto dirty = juce::Rectangle<int>::leftTopRightBottom(left, top, right, bottom)
                                       .getIntersection(getLocalBounds());
                if (!dirty.isEmpty()) {
                    repaint(dirty);
                }
            }

        private:
            static constexpr float kQuarterCircleK = 0.5522847498f; // (4/3) * (sqrt(2) - 1)

            enum class FillMode {
                kEmpty,
                kFull,
                kRect,
                kPath
            };

            static void addClippedRoundedRect(juce::Path& path,
                                              const juce::Rectangle<float> track,
                                              const float fill_width,
                                              const float csx, const float csy) {
                const auto x0 = track.getX();
                const auto y0 = track.getY();
                const auto x1 = track.getRight();
                const auto y1 = track.getBottom();
                const auto x_cut = x0 + fill_width;

                if (fill_width < csx) {
                    const auto cos_a = std::clamp(1.f - fill_width / csx, 0.f, 1.f);
                    const auto alpha = std::acos(cos_a);
                    const auto sin_a = std::sqrt(std::max(0.f, 1.f - cos_a * cos_a));
                    const auto k = (4.f / 3.f) * std::tan(alpha * 0.25f);
                    const auto y_top = y0 + csy * (1.f - sin_a);
                    const auto y_bot = y1 - csy * (1.f - sin_a);

                    path.startNewSubPath(x0, y0 + csy);
                    path.cubicTo(x0, y0 + csy * (1.f - k),
                                 x_cut - k * csx * sin_a, y_top + k * csy * cos_a,
                                 x_cut, y_top);
                    path.lineTo(x_cut, y_bot);
                    path.cubicTo(x_cut - k * csx * sin_a, y_bot - k * csy * cos_a,
                                 x0, y1 - csy * (1.f - k),
                                 x0, y1 - csy);
                    path.closeSubPath();
                    return;
                }

                const auto one_minus_k90 = 1.f - kQuarterCircleK;
                path.startNewSubPath(x0, y0 + csy);
                path.cubicTo(x0, y0 + csy * one_minus_k90,
                             x0 + csx * one_minus_k90, y0,
                             x0 + csx, y0);

                if (fill_width <= track.getWidth() - csx) {
                    if (x_cut > x0 + csx) {
                        path.lineTo(x_cut, y0);
                    }
                    path.lineTo(x_cut, y1);
                    if (x_cut > x0 + csx) {
                        path.lineTo(x0 + csx, y1);
                    }
                } else {
                    const auto right_arc_x = x1 - csx;
                    if (right_arc_x > x0 + csx) {
                        path.lineTo(right_arc_x, y0);
                    }
                    const auto sin_b = std::clamp((x_cut - right_arc_x) / csx, 0.f, 1.f);
                    const auto beta = std::asin(sin_b);
                    const auto cos_b = std::sqrt(std::max(0.f, 1.f - sin_b * sin_b));
                    const auto k = (4.f / 3.f) * std::tan(beta * 0.25f);
                    const auto y_top = y0 + csy * (1.f - cos_b);
                    const auto y_bot = y1 - csy * (1.f - cos_b);

                    path.cubicTo(right_arc_x + k * csx, y0,
                                 x_cut - k * csx * cos_b, y_top - k * csy * sin_b,
                                 x_cut, y_top);
                    path.lineTo(x_cut, y_bot);
                    path.cubicTo(x_cut - k * csx * cos_b, y_bot + k * csy * sin_b,
                                 right_arc_x + k * csx, y1,
                                 right_arc_x, y1);
                    if (right_arc_x > x0 + csx) {
                        path.lineTo(x0 + csx, y1);
                    }
                }

                path.cubicTo(x0 + csx * one_minus_k90, y1,
                             x0, y1 - csy * one_minus_k90,
                             x0, y1 - csy);
                path.closeSubPath();
            }

            void updateFillGeometry() {
                const auto track_width = track_bound_.getWidth();
                const auto fill_width = std::clamp(value_, 0.f, 1.f) * track_width;
                fill_bound_ = track_bound_.withWidth(fill_width);

                if (track_bound_.isEmpty() || fill_width <= 0.f) {
                    fill_mode_ = FillMode::kEmpty;
                } else if (corner_x_ <= 0.f || corner_y_ <= 0.f) {
                    fill_mode_ = FillMode::kRect;
                } else if (fill_width >= track_width) {
                    fill_mode_ = FillMode::kFull;
                } else {
                    fill_mode_ = FillMode::kPath;
                    fill_path_.clear();
                    addClippedRoundedRect(fill_path_, track_bound_, fill_width, corner_x_, corner_y_);
                }
            }

            UIBase& base_;
            float value_{0.f}, marker_width_{0.f}, marker_extension_{0.f};
            float corner_size_{0.f}, corner_x_{0.f}, corner_y_{0.f};
            bool hovered_{false};
            FillMode fill_mode_{FillMode::kEmpty};
            juce::Rectangle<float> track_bound_, fill_bound_;
            juce::Path fill_path_;
        };

    public:
        explicit CompactLinearSlider(const juce::String& label_text, UIBase& base,
                                     const juce::String& tooltip_text = "") :
            base_(base), background_(base_), display_(base_),
            slider_(base_),
            name_look_and_feel_(base_), text_look_and_feel_(base_) {
            juce::ignoreUnused(base_);

            slider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider_.setTextBoxIsEditable(false);
            slider_.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            slider_.setSliderSnapsToMousePosition(false);
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
            text_.setBorderSize(juce::BorderSize<int>{0});
            text_.setInterceptsMouseClicks(false, false);
            text_.addListener(this);
            addAndMakeVisible(text_);

            // setup label
            if constexpr (kUseName) {
                text_.setVisible(false);
                label_.setText(label_text, juce::dontSendNotification);
                label_.setJustificationType(juce::Justification::centred);
                label_.setLookAndFeel(&name_look_and_feel_);
                label_.setBorderSize(juce::BorderSize<int>{0});
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
            slider_.setBounds(bound);
            auto text_bound = bound;
            auto track_bound = bound;
            auto display_bound = bound;
            auto marker_width = 0;
            auto marker_extension = 0;
            if constexpr (kUseName || kUseDisplay) {
                if (base_.getUIStyle() == UIStyle::kFlat) {
                    const auto font_size = base_.getFontSize();
                    track_bound = getFlatTrackBounds(bound, font_size);
                    if constexpr (kUseDisplay) {
                        marker_width = getFlatMarkerWidth(font_size, bound.getWidth());
                        marker_extension = getFlatMarkerExtension(font_size);
                        display_bound = bound.getIntersection(
                            track_bound.expanded((marker_width + 1) / 2, marker_extension));
                    }
                    if constexpr (kUseName) {
                        text_bound = bound.withBottom(track_bound.getY());
                    }
                }
            }
            if constexpr (kUseBackground) {
                background_.setBounds(track_bound);
            }
            if constexpr (kUseDisplay) {
                display_.setBounds(display_bound);
                display_.setTrackBounds(track_bound - display_bound.getPosition(), marker_width, marker_extension);
            }
            text_.setBounds(text_bound);
            if constexpr (kUseName) {
                label_.setBounds(text_bound);
            }
        }

        static int getFlatMarkerExtension(const float font_size) {
            return juce::roundToInt(font_size * kFlatMarkerExtension);
        }

        static juce::Rectangle<int> getFlatTrackBounds(juce::Rectangle<int> bound, const float font_size) {
            const auto bar_thickness = std::max(juce::roundToInt(font_size * kFlatBarThickness), 2);
            if constexpr (kUseDisplay) {
                bound.reduce((getFlatMarkerWidth(font_size, bound.getWidth()) + 1) / 2, 0);
            }
            bound.removeFromBottom(getFlatMarkerExtension(font_size));
            return bound.removeFromBottom(bar_thickness);
        }

        void mouseUp(const juce::MouseEvent& event) override {
            slider_.mouseUp(event);
        }

        void mouseDown(const juce::MouseEvent& event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
                return;
            }
            slider_.mouseDown(event);
            updateDragDistance(event.mods.isShiftDown());
        }

        void mouseDrag(const juce::MouseEvent& event) override {
            if (event.mods.isRightButtonDown()) {
                return;
            }
            slider_.mouseDrag(event);
        }

        void mouseEnter(const juce::MouseEvent& event) override {
            slider_.mouseEnter(event);
            if constexpr (kUseDisplay) {
                display_.setHovered(true);
            }
            if constexpr (kUseName) {
                text_.setVisible(true);
                label_.setVisible(false);
            }
        }

        void mouseExit(const juce::MouseEvent& event) override {
            slider_.mouseExit(event);
            if constexpr (kUseDisplay) {
                display_.setHovered(false);
            }
            if (text_.getCurrentTextEditor() != nullptr) {
                return;
            }
            if constexpr (kUseName) {
                text_.setVisible(false);
                label_.setVisible(true);
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

        inline SnappingSlider& getSlider() { return slider_; }

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

        void setMouseDragSensitivity(const int x) {
            drag_distance_ = x;
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

        int drag_distance_{10};

        void lookAndFeelChanged() override {
            resized();
        }

        juce::String getDisplayValue(const juce::Slider& s) const {
            const auto value = s.getValue();
            if (value_formatter_) {
                return value_formatter_(value);
            }
            if (std::abs(value) < 1e-5) {
                return {"0"};
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

        void labelTextChanged(juce::Label*) override {
        }

        void editorShown(juce::Label*, juce::TextEditor& editor) override {
            editor.setInterceptsMouseClicks(false, false);
            editor.setInputRestrictions(0, permitted_characters_);
            text_.addMouseListener(this, true);

            if constexpr (kUseName) {
                text_.setVisible(true);
                label_.setVisible(false);
            }

            editor.setJustification(juce::Justification::centred);
            editor.setIndents(2, 0);
            editor.setBorder(juce::BorderSize<int>{0});
            editor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            editor.setColour(juce::TextEditor::focusedOutlineColourId, base_.getTextColour().withAlpha(.5f));
            editor.setColour(juce::TextEditor::highlightedTextColourId, base_.getTextColour());

            const juce::FontOptions font_opt{base_.getFontSize() * font_scale_};
            editor.setFont(font_opt);
            editor.applyFontToAllText(font_opt);
            editor.applyColourToAllText(base_.getTextColour(), true);
        }

        void editorHidden(juce::Label*, juce::TextEditor& editor) override {
            text_.removeMouseListener(this);
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

            slider_.setValueWithGesture(actual_value);
            if constexpr (kUseName) {
                if (!isMouseOver(true)) {
                    text_.setVisible(false);
                    label_.setVisible(true);
                }
            }
        }

        void sliderValueChanged(juce::Slider*) override {
            text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
            display_.setSliderValue(
                static_cast<float>(slider_.getNormalisableRange().convertTo0to1(slider_.getValue())));
        }

        void updateDragDistance(const bool is_shift_pressed) {
            int actual_drag_distance;
            if (is_shift_pressed) {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) * 10.f / base_.getSensitivity(SensitivityIdx::kMouseSliderFine));
            } else {
                actual_drag_distance = juce::roundToInt(
                    static_cast<float>(drag_distance_) / base_.getSensitivity(SensitivityIdx::kMouseSlider));
            }
            actual_drag_distance = std::max(actual_drag_distance, 1);
            slider_.setMouseDragSensitivity(actual_drag_distance);
        }
    };
}
