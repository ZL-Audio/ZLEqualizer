// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "two_value_rotary_slider.hpp"

namespace zlInterface {
    TwoValueRotarySlider::TwoValueRotarySlider(const juce::String &labelText, UIBase &base)
        : uiBase(base),
          slider1LAF(base), slider2LAF(base),
          slider1(base), slider2(base),
          labelLookAndFeel(base), labelLookAndFeel1(base), labelLookAndFeel2(base), textBoxLAF(base),
          animator{} {
        for (auto const s: {&slider1, &slider2}) {
            s->setSliderStyle(uiBase.getRotaryStyle());
            s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            s->setDoubleClickReturnValue(true, 0.0);
            s->setScrollWheelEnabled(true);
            s->setInterceptsMouseClicks(false, false);
        }

        slider1.setLookAndFeel(&slider1LAF);
        slider2LAF.setEditable(showSlider2.load());
        slider2.setLookAndFeel(&slider2LAF);
        slider1.addListener(this);
        slider2.addListener(this);

        addAndMakeVisible(slider1);
        addAndMakeVisible(slider2);

        label.setText(labelText, juce::dontSendNotification);
        label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        label2.setText(getDisplayValue(slider2), juce::dontSendNotification);

        labelLookAndFeel.setFontScale(1.75f);
        labelLookAndFeel1.setFontScale(FontHuge);
        labelLookAndFeel1.setJustification(juce::Justification::centredBottom);
        labelLookAndFeel1.setAlpha(0.f);
        labelLookAndFeel2.setFontScale(FontHuge);
        labelLookAndFeel2.setJustification(juce::Justification::centredTop);
        labelLookAndFeel2.setAlpha(0.f);

        label.setLookAndFeel(&labelLookAndFeel);
        label1.setLookAndFeel(&labelLookAndFeel1);
        label2.setLookAndFeel(&labelLookAndFeel2);

        for (auto &l: {&label, &label1, &label2}) {
            l->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(l);
        }

        setEditable(true);
        setShowSlider2(false);
        label1.setEditable(false, true);
        label2.setEditable(false, true);

        label1.setJustificationType(juce::Justification::centred);
        label2.setJustificationType(juce::Justification::centred);
        label1.addListener(this);
        label2.addListener(this);
    }

    TwoValueRotarySlider::~TwoValueRotarySlider() {
        animator.cancelAllAnimations(false);
        slider1.removeListener(this);
        slider2.removeListener(this);
        label1.removeListener(this);
        label2.removeListener(this);
    }

    juce::String TwoValueRotarySlider::getDisplayValue(juce::Slider &s) {
        const auto interval = s.getNormalisableRange().interval;
        auto value = std::round(s.getValue() / interval) * interval;
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

    void TwoValueRotarySlider::setShowSlider2(const bool x) {
        showSlider2.store(x);

        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad.load(),
                                            bound.getHeight() - ubPad.load());

        auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.6f,
                                                      bound.getHeight() * 0.5f);
        if (showSlider2.load()) {
            slider2LAF.setEditable(true);
            const auto valueBound1 = labelBound.removeFromTop(labelBound.getHeight() * 0.5f);
            const auto valueBound2 = labelBound;
            label1.setBounds(valueBound1.toNearestInt());
            label2.setBounds(valueBound2.toNearestInt());
            labelLookAndFeel1.setJustification(juce::Justification::centredBottom);
            labelLookAndFeel2.setJustification(juce::Justification::centredTop);
        } else {
            slider2LAF.setEditable(false);
            labelBound = labelBound.withSizeKeepingCentre(labelBound.getWidth(), labelBound.getHeight() * .5f);
            label1.setBounds(labelBound.toNearestInt());
            label2.setBounds(0, 0, 0, 0);
            labelLookAndFeel1.setJustification(juce::Justification::centred);
        }
    }

    void TwoValueRotarySlider::mouseUp(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
            return;
        }
        if (!showSlider2.load() || event.mods.isLeftButtonDown()) {
            slider1.mouseUp(event);
        } else {
            slider2.mouseUp(event);
        }
    }

    void TwoValueRotarySlider::mouseDown(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown()) {
            return;
        }
        if (!showSlider2.load() || event.mods.isLeftButtonDown()) {
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

    void TwoValueRotarySlider::mouseDrag(const juce::MouseEvent &event) {
        if (!showSlider2.load() || event.mods.isLeftButtonDown()) {
            slider1.mouseDrag(event);
        } else {
            slider2.mouseDrag(event);
        }
    }

    void TwoValueRotarySlider::mouseEnter(const juce::MouseEvent &event) {
        slider1.mouseEnter(event);
        slider2.mouseEnter(event);
        labelLookAndFeel.setAlpha(0.f);
        labelLookAndFeel1.setAlpha(1.f);
        labelLookAndFeel2.setAlpha(1.f);
        label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        animator.cancelAnimation(animationId, false);
        repaint();
    }

    void TwoValueRotarySlider::mouseExit(const juce::MouseEvent &event) {
        slider1.mouseExit(event);
        slider2.mouseExit(event);

        if (label1.getCurrentTextEditor() != nullptr || label2.getCurrentTextEditor() != nullptr) {
            return;
        }

        leaveAnimation();
    }

    void TwoValueRotarySlider::mouseMove(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
    }

    void TwoValueRotarySlider::mouseDoubleClick(const juce::MouseEvent &event) {
        if (uiBase.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
            const auto portion = static_cast<float>(event.getPosition().getY()
                ) / static_cast<float>(getLocalBounds().getHeight());
            if (portion < .5f || !showSlider2.load()) {
                label1.showEditor();
                return;
            } else {
                label2.showEditor();
                return;
            }
        }
        if (!showSlider2.load() || event.mods.isLeftButtonDown()) {
            slider1.mouseDoubleClick(event);
        } else {
            slider2.mouseDoubleClick(event);
        }
    }

    void TwoValueRotarySlider::mouseWheelMove(const juce::MouseEvent &event,
                                              const juce::MouseWheelDetails &wheel) {
        if (!showSlider2.load()) {
            slider1.mouseWheelMove(event, wheel);
        } else {
            slider1.mouseWheelMove(event, wheel);
            slider2.mouseWheelMove(event, wheel);
        }
    }

    void TwoValueRotarySlider::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad.load(),
                                            bound.getHeight() - ubPad.load());
        slider1.setBounds(bound.toNearestInt());
        slider2.setBounds(bound.toNearestInt());

        auto labelBound = bound.withSizeKeepingCentre(bound.getWidth() * 0.7f,
                                                      bound.getHeight() * 0.6f);
        label.setBounds(labelBound.toNearestInt());

        setShowSlider2(showSlider2.load());
    }

    void TwoValueRotarySlider::labelTextChanged(juce::Label *labelThatHasChanged) {
        juce::ignoreUnused(labelThatHasChanged);
    }

    void TwoValueRotarySlider::editorShown(juce::Label *l, juce::TextEditor &editor) {
        juce::ignoreUnused(l);
        editor.setInputRestrictions(0, "-0123456789.kK");

        labelLookAndFeel.setAlpha(0);
        labelLookAndFeel1.setAlpha(1);
        labelLookAndFeel2.setAlpha(1);

        for (auto &ll: {&label, &label1, &label2}) {
            ll->repaint();
        }
        editor.setJustification(juce::Justification::centred);
        editor.setColour(juce::TextEditor::outlineColourId, uiBase.getTextColor());
        editor.setColour(juce::TextEditor::highlightedTextColourId, uiBase.getTextColor());
#if (USE_JUCE7_INSTEAD_OF_LATEST)
        editor.applyFontToAllText(uiBase.getFontSize() * FontHuge);
#else
        editor.applyFontToAllText(juce::FontOptions{uiBase.getFontSize() * FontHuge});
#endif
        editor.applyColourToAllText(uiBase.getTextColor(), true);
    }

    void TwoValueRotarySlider::editorHidden(juce::Label *l, juce::TextEditor &editor) {
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

        leaveAnimation();
    }

    void TwoValueRotarySlider::sliderValueChanged(juce::Slider *slider) {
        if (slider == &slider1) {
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
            label1.repaint();
        }
        if (slider == &slider2) {
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }

    void TwoValueRotarySlider::leaveAnimation() {
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto frizEffect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {1.5f}, {0.f}, 1000, friz::Parametric::kLinear)
        };
        frizEffect->updateFn = [this](int, const auto &vals) {
            auto val = juce::jmin(vals[0], 1.0f);
            labelLookAndFeel.setAlpha(1 - val);
            labelLookAndFeel1.setAlpha(val);
            labelLookAndFeel2.setAlpha(val);
            for (auto &l: {&label, &label1, &label2}) {
                l->repaint();
            }
        };

        animator.addAnimation(std::move(frizEffect));
    }
}
