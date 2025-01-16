// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_linear_slider.hpp"

namespace zlInterface {
    CompactLinearSlider::CompactLinearSlider(const juce::String &labelText, UIBase &base)
        : uiBase(base),
          sliderLookAndFeel(base), nameLookAndFeel(base), textLookAndFeel(base),
          slider(base) {
        juce::ignoreUnused(uiBase);

        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        sliderLookAndFeel.setTextAlpha(0.f);
        slider.setTextBoxIsEditable(false);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        slider.setDoubleClickReturnValue(true, 0.0);
        slider.setScrollWheelEnabled(true);
        slider.setInterceptsMouseClicks(false, false);
        slider.setLookAndFeel(&sliderLookAndFeel);
        slider.addListener(this);
        addAndMakeVisible(slider);

        text.setText(getDisplayValue(slider), juce::dontSendNotification);
        text.setJustificationType(juce::Justification::centred);
        textLookAndFeel.setAlpha(0.f);
        textLookAndFeel.setFontScale(FontHuge);
        text.setLookAndFeel(&textLookAndFeel);
        text.setInterceptsMouseClicks(false, false);
        text.addListener(this);
        addAndMakeVisible(text);

        // setup label
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setLookAndFeel(&nameLookAndFeel);
        nameLookAndFeel.setFontScale(FontHuge);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        setEditable(true);
    }

    CompactLinearSlider::~CompactLinearSlider() {
        animator.cancelAllAnimations(false);
    }

    void CompactLinearSlider::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - lrPad.load(),
                                            uiBase.getFontSize() * FontLarge * 1.75f - ubPad.load());
        slider.setBounds(bound.toNearestInt());
        text.setBounds(bound.toNearestInt());
        label.setBounds(bound.toNearestInt());
    }

    void CompactLinearSlider::mouseUp(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
            return;
        }
        slider.mouseUp(event);
    }

    void CompactLinearSlider::mouseDown(const juce::MouseEvent &event) {
        if (event.getNumberOfClicks() > 1 || event.mods.isCommandDown() || event.mods.isRightButtonDown()) {
            return;
        }
        slider.mouseDown(event);
    }

    void CompactLinearSlider::mouseDrag(const juce::MouseEvent &event) {
        if (event.mods.isRightButtonDown()) {
            return;
        }
        slider.mouseDrag(event);
    }

    void CompactLinearSlider::mouseEnter(const juce::MouseEvent &event) {
        textLookAndFeel.setAlpha(1.f);
        nameLookAndFeel.setAlpha(0.f);
        slider.mouseEnter(event);
        animator.cancelAnimation(animationId, false);
        text.repaint();
        label.repaint();
    }

    void CompactLinearSlider::mouseExit(const juce::MouseEvent &event) {
        sliderLookAndFeel.setTextAlpha(1.f);
        nameLookAndFeel.setAlpha(0.f);
        slider.mouseExit(event);

        if (text.getCurrentTextEditor() != nullptr) {
            return;
        }

        leaveAnimation();
    }

    void CompactLinearSlider::mouseMove(const juce::MouseEvent &event) {
        slider.mouseMove(event);
    }

    void CompactLinearSlider::mouseDoubleClick(const juce::MouseEvent &event) {
        if (uiBase.getIsSliderDoubleClickOpenEditor() != event.mods.isCommandDown()) {
            text.showEditor();
        } else {
            slider.mouseDoubleClick(event);
        }
    }

    void CompactLinearSlider::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        slider.mouseWheelMove(event, wheel);
    }

    void CompactLinearSlider::labelTextChanged(juce::Label *labelThatHasChanged) {
        juce::ignoreUnused(labelThatHasChanged);
    }

    void CompactLinearSlider::editorShown(juce::Label *l, juce::TextEditor &editor) {
        editor.setInterceptsMouseClicks(false, false);
        juce::ignoreUnused(l);
        editor.setInputRestrictions(0, "-0123456789.kK");
        text.addMouseListener(this, true);

        editor.setJustification(juce::Justification::centred);
        editor.setColour(juce::TextEditor::outlineColourId, uiBase.getTextColor());
        editor.setColour(juce::TextEditor::highlightedTextColourId, uiBase.getTextColor());
#if (JUCE_MAJOR_VERSION < 8)
        editor.applyFontToAllText(uiBase.getFontSize() * FontHuge);
#else
        editor.applyFontToAllText(juce::FontOptions{uiBase.getFontSize() * FontHuge});
#endif
        editor.applyColourToAllText(uiBase.getTextColor(), true);
    }

    void CompactLinearSlider::editorHidden(juce::Label *l, juce::TextEditor &editor) {
        juce::ignoreUnused(l);
        text.removeMouseListener(this);
        auto k = 1.0;
        const auto ctext = editor.getText();
        if (ctext.contains("k") || ctext.contains("K")) {
            k = 1000.0;
        }
        const auto actualValue = ctext.getDoubleValue() * k;

        slider.setValue(actualValue, juce::sendNotificationAsync);

        leaveAnimation();
    }

    void CompactLinearSlider::sliderValueChanged(juce::Slider *) {
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
        text.repaint();
    }

    juce::String CompactLinearSlider::getDisplayValue(juce::Slider &s) {
        auto value = s.getValue();
        juce::String labelToDisplay = juce::String(s.getTextFromValue(value)).substring(0, 4);
        if (value < 10000 && labelToDisplay.contains(".")) {
            labelToDisplay = juce::String(value).substring(0, 5);
        }
        if (value > 10000) {
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

    void CompactLinearSlider::leaveAnimation() {
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto frizEffect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {1.5f}, {0.f}, 1000, friz::Parametric::kLinear)
        };
        frizEffect->updateFn = [this](int, const auto &vals) {
            auto val = juce::jmin(vals[0], 1.0f);
            textLookAndFeel.setAlpha(val);
            nameLookAndFeel.setAlpha(1.f - val);
            text.repaint();
            label.repaint();
        };
        animator.addAnimation(std::move(frizEffect));
    }
}
