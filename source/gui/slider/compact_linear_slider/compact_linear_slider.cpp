// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
        addAndMakeVisible(slider);

        text.setText(getDisplayValue(slider), juce::dontSendNotification);
        textLookAndFeel.setAlpha(0.f);
        textLookAndFeel.setFontScale(FontHuge);
        text.setLookAndFeel(&textLookAndFeel);
        text.setInterceptsMouseClicks(false, false);
        // text.setEditable(false, true);
        // text.addMouseListener(this, false);
        text.addListener(this);
        addAndMakeVisible(text);

        // setup label
        label.setText(labelText, juce::dontSendNotification);
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
        slider.mouseUp(event);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
    }

    void CompactLinearSlider::mouseDown(const juce::MouseEvent &event) {
        slider.mouseDown(event);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
    }

    void CompactLinearSlider::mouseDrag(const juce::MouseEvent &event) {
        slider.mouseDrag(event);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
    }

    void CompactLinearSlider::mouseEnter(const juce::MouseEvent &event) {
        textLookAndFeel.setAlpha(1.f);
        nameLookAndFeel.setAlpha(0.f);
        slider.mouseEnter(event);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
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

    void CompactLinearSlider::mouseMove(const juce::MouseEvent &event) {
        slider.mouseMove(event);
    }

    void CompactLinearSlider::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown()) {
            text.showEditor();
        } else {
            slider.mouseDoubleClick(event);
            text.setText(getDisplayValue(slider), juce::dontSendNotification);
        }
    }

    void CompactLinearSlider::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        slider.mouseWheelMove(event, wheel);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
    }

    void CompactLinearSlider::labelTextChanged(juce::Label *labelThatHasChanged) {
        juce::ignoreUnused(labelThatHasChanged);
    }

    void CompactLinearSlider::editorShown(juce::Label *l, juce::TextEditor &editor) {
        juce::ignoreUnused(l);
        editor.setInputRestrictions(0, "-0123456789.kK");\

        text.repaint();
        label.repaint();
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

    void CompactLinearSlider::editorHidden(juce::Label *l, juce::TextEditor &editor) {
        juce::ignoreUnused(l);

        auto k = 1.0;
        const auto ctext = editor.getText();
        if (ctext.contains("k") || ctext.contains("K")) {
            k = 1000.0;
        }
        const auto actualValue = ctext.getDoubleValue() * k;
        // juce::FileLogger logger{juce::File{"/Volumes/Ramdisk/log.txt"}, ""};
        // logger.logMessage(juce::String(actualValue));
        slider.setValue(actualValue, juce::sendNotificationAsync);
        text.repaint();
        label.repaint();
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
        return labelToDisplay;
    }
}
