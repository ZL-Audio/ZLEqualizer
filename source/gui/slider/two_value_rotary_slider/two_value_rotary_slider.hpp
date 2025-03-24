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

namespace zlInterface {
    template<bool Opaque = true, bool UseSecondSlider = true>
    class TwoValueRotarySlider final : public juce::Component,
                                       private juce::Label::Listener,
                                       private juce::Slider::Listener,
                                       public juce::SettableTooltipClient {
    public:
        static constexpr float startAngle = 2.0943951023931953f, endAngle = 7.3303828583761845f;

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
                                      {.fit = false, .drawBright = false, .drawDark = true});
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
        explicit TwoValueRotarySlider(const juce::String &labelText, UIBase &base,
                                      multilingual::labels labelIdx = multilingual::labels::labelNum)
            : uiBase(base), background(uiBase),
              slider1(base, labelText), slider2(base, labelText), display(base),
              labelLookAndFeel(base), labelLookAndFeel1(base), labelLookAndFeel2(base), textBoxLAF(base) {
            addAndMakeVisible(background);
            for (auto const s: {&slider1, &slider2}) {
                s->setSliderStyle(uiBase.getRotaryStyle());
                s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
                s->setDoubleClickReturnValue(true, 0.0);
                s->setScrollWheelEnabled(true);
                s->setInterceptsMouseClicks(false, false);
            }

            slider1.addListener(this);
            if (UseSecondSlider) {
                slider2.addListener(this);
            }
            addAndMakeVisible(display);

            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.setBufferedToImage(true);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);

            labelLookAndFeel.setFontScale(1.75f);
            labelLookAndFeel1.setFontScale(FontHuge);
            label1.setJustificationType(juce::Justification::centredBottom);

            label.setLookAndFeel(&labelLookAndFeel);
            label1.setLookAndFeel(&labelLookAndFeel1);

            if (UseSecondSlider) {
                label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
                labelLookAndFeel2.setFontScale(FontHuge);
                label2.setJustificationType(juce::Justification::centredTop);
                label2.setLookAndFeel(&labelLookAndFeel2);
            }

            for (auto &l: {&label, &label1, &label2}) {
                l->setInterceptsMouseClicks(false, false);
            }

            addAndMakeVisible(label);
            addChildComponent(label1);

            setEditable(true);
            label1.setEditable(false, true);

            label1.addListener(this);

            if (UseSecondSlider) {
                addChildComponent(label2);
                label2.setEditable(false, true);
                label2.addListener(this);
            }

            if (labelIdx != multilingual::labels::labelNum) {
                SettableTooltipClient::setTooltip(uiBase.getToolTipText(labelIdx));
            }

            setOpaque(Opaque);
        }

        ~TwoValueRotarySlider() override {
            slider1.removeListener(this);
            label1.removeListener(this);
            if (UseSecondSlider) {
                slider2.removeListener(this);
                label2.removeListener(this);
            }
        }

        void paint(juce::Graphics &g) override {
            if (Opaque) {
                g.fillAll(uiBase.getBackgroundColor());
            }
        }

        void mouseUp(const juce::MouseEvent &event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (!showSlider2 || event.mods.isLeftButtonDown()) {
                slider1.mouseUp(event);
            } else {
                slider2.mouseUp(event);
            }
        }

        void mouseDown(const juce::MouseEvent &event) override {
            if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
                return;
            }
            if (!showSlider2 || event.mods.isLeftButtonDown()) {
                slider1.mouseDown(event);
            } else {
                slider2.mouseDown(event);
            }
            const auto currentShiftPressed = event.mods.isShiftDown();
            if (currentShiftPressed != isShiftPressed) {
                isShiftPressed = currentShiftPressed;
                updateDragDistance();
            }
        }

        void mouseDrag(const juce::MouseEvent &event) override {
            if (!showSlider2 || event.mods.isLeftButtonDown()) {
                slider1.mouseDrag(event);
            } else {
                slider2.mouseDrag(event);
            }
        }

        void mouseEnter(const juce::MouseEvent &event) override {
            slider1.mouseEnter(event);
            slider2.mouseEnter(event);
            label.setVisible(false);
            label1.setVisible(true);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
            if (showSlider2) {
                label2.setVisible(true);
                label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
            }
        }

        void mouseExit(const juce::MouseEvent &event) override {
            slider1.mouseExit(event);
            slider2.mouseExit(event);

            if (label1.getCurrentTextEditor() != nullptr || label2.getCurrentTextEditor() != nullptr) {
                return;
            }

            label.setVisible(true);
            label1.setVisible(false);
            if (showSlider2) {
                label2.setVisible(false);
            }
        }

        void mouseMove(const juce::MouseEvent &event) override {
            juce::ignoreUnused(event);
        }

        void mouseDoubleClick(const juce::MouseEvent &event) override {
            if (uiBase.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
                const auto portion = static_cast<float>(event.getPosition().getY()
                                     ) / static_cast<float>(getLocalBounds().getHeight());
                if (portion < .5f || !showSlider2) {
                    label1.showEditor();
                    return;
                } else {
                    label2.showEditor();
                    return;
                }
            }
            if (!showSlider2 || event.mods.isLeftButtonDown()) {
                slider1.mouseDoubleClick(event);
            } else {
                slider2.mouseDoubleClick(event);
            }
        }

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override {
            if (!showSlider2) {
                slider1.mouseWheelMove(event, wheel);
            } else {
                slider1.mouseWheelMove(event, wheel);
                slider2.mouseWheelMove(event, wheel);
            }
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad,
                                                bound.getHeight() - ubPad);
            background.setBounds(bound.toNearestInt());
            slider1.setBounds(bound.toNearestInt());
            slider2.setBounds(bound.toNearestInt());
            display.setBounds(bound.toNearestInt());

            auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.7f,
                                                          bound.getHeight() * 0.6f);
            label.setBounds(labelBound.toNearestInt());

            setShowSlider2(showSlider2);
        }

        inline juce::Slider &getSlider1() { return slider1; }

        inline juce::Slider &getSlider2() { return slider2; }

        void setShowSlider2(const bool x) {
            showSlider2 = x && UseSecondSlider;

            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad,
                                                bound.getHeight() - ubPad);

            auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.6f,
                                                          bound.getHeight() * 0.5f);
            if (showSlider2) {
                const auto valueBound1 = labelBound.removeFromTop(labelBound.getHeight() * 0.5f);
                const auto valueBound2 = labelBound;
                label1.setBounds(valueBound1.toNearestInt());
                label2.setBounds(valueBound2.toNearestInt());
                label1.setJustificationType(juce::Justification::centredBottom);
                label2.setJustificationType(juce::Justification::centredTop);
            } else {
                labelBound = labelBound.withSizeKeepingCentre(labelBound.getWidth(), labelBound.getHeight() * .5f);
                label1.setBounds(labelBound.toNearestInt());
                label2.setVisible(false);
                label1.setJustificationType(juce::Justification::centred);
            }
            display.setShowSlider2(showSlider2);
        }

        inline void setPadding(const float lr, const float ub) {
            lrPad = lr;
            ubPad = ub;
        }

        inline void setEditable(const bool x) {
            editable = x;
            labelLookAndFeel.setEditable(x);
            labelLookAndFeel1.setEditable(x);
            labelLookAndFeel2.setEditable(x);
            setInterceptsMouseClicks(x, false);
            label.repaint();
        }

        inline bool getEditable() const { return editable; }

        void setMouseDragSensitivity(const int x) {
            dragDistance = x;
            updateDragDistance();
        }

        void updateDisplay() {
            display.setSlider1Value(
                    static_cast<float>(slider1.getNormalisableRange().convertTo0to1(slider1.getValue())));
            display.setSlider2Value(
                    static_cast<float>(slider2.getNormalisableRange().convertTo0to1(slider2.getValue())));
        }

    private:
        UIBase &uiBase;
        Background background;

        SnappingSlider slider1, slider2;

        Display display;

        NameLookAndFeel labelLookAndFeel, labelLookAndFeel1, labelLookAndFeel2;
        NameLookAndFeel textBoxLAF;

        juce::Label label, label1, label2;

        bool showSlider2{false}, editable{true};
        float lrPad{0.f}, ubPad{0.f};

        int dragDistance{10};
        bool isShiftPressed{false};

        static juce::String getDisplayValue(juce::Slider &s) {
            auto value = s.getNormalisableRange().snapToLegalValue(s.getValue());
            juce::String labelToDisplay = juce::String(value).substring(0, 4);
            if (value < 10000 && labelToDisplay.contains(".")) {
                labelToDisplay = juce::String(value).substring(0, 5);
            }
            if (value >= 10000) {
                value = value / 1000;
                labelToDisplay = juce::String(value).substring(0, 4) + "K";
            }
            // remove trailing zeros
            while (labelToDisplay.contains(".")) {
                const auto lastS = labelToDisplay.getLastCharacter();
                if (lastS == '.' || lastS == '0') {
                    labelToDisplay = labelToDisplay.dropLastCharacters(1);
                } else {
                    break;
                }
            }
            return labelToDisplay;
        }

        void labelTextChanged(juce::Label *labelThatHasChanged) override {
            juce::ignoreUnused(labelThatHasChanged);
        }

        void editorShown(juce::Label *l, juce::TextEditor &editor) override {
            juce::ignoreUnused(l);
            editor.setInputRestrictions(0, "-0123456789.kK");

            label.setVisible(false);
            label1.setVisible(true);
            if (showSlider2) {
                label2.setVisible(true);
            }

            editor.setJustification(juce::Justification::centred);
            editor.setColour(juce::TextEditor::outlineColourId, uiBase.getTextColor());
            editor.setColour(juce::TextEditor::highlightedTextColourId, uiBase.getTextColor());
            editor.applyFontToAllText(juce::FontOptions{uiBase.getFontSize() * FontHuge});
            editor.applyColourToAllText(uiBase.getTextColor(), true);
        }

        void editorHidden(juce::Label *l, juce::TextEditor &editor) override {
            auto k = 1.0;
            const auto text = editor.getText();
            if (text.contains("k") || text.contains("K")) {
                k = 1000.0;
            }
            const auto actualValue = text.getDoubleValue() * k;

            if (l == &label1) {
                slider1.setValue(actualValue, juce::sendNotificationAsync);
            }
            if (l == &label2) {
                slider2.setValue(actualValue, juce::sendNotificationAsync);
            }

            label.setVisible(true);
            label1.setVisible(false);
            if (showSlider2) {
                label2.setVisible(false);
            }
        }

        void sliderValueChanged(juce::Slider *slider) override {
            if (slider == &slider1) {
                label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
                display.setSlider1Value(
                    static_cast<float>(slider1.getNormalisableRange().convertTo0to1(slider1.getValue())));
            }
            if (slider == &slider2) {
                label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
                display.setSlider2Value(
                    static_cast<float>(slider2.getNormalisableRange().convertTo0to1(slider2.getValue())));
            }
        }

        void updateDragDistance() {
            int actualDragDistance;
            if (isShiftPressed) {
                actualDragDistance = juce::roundToInt(
                    static_cast<float>(dragDistance) / uiBase.getSensitivity(sensitivityIdx::mouseDragFine));
            } else {
                actualDragDistance = juce::roundToInt(
                    static_cast<float>(dragDistance) / uiBase.getSensitivity(sensitivityIdx::mouseDrag));
            }
            actualDragDistance = std::max(actualDragDistance, 1);
            slider1.setMouseDragSensitivity(actualDragDistance);
            slider2.setMouseDragSensitivity(actualDragDistance);
        }
    };
}
