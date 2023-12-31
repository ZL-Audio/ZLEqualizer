//
// Created by Zishu Liu on 12/28/23.
//

#include "two_value_rotary_slider.h"

namespace zlInterface {
    TwoValueRotarySlider::TwoValueRotarySlider(const juce::String &labelText, UIBase &base)
        : uiBase(base), slider1LAF(base), slider2LAF(base),
          labelLookAndFeel(base), labelLookAndFeel1(base), labelLookAndFeel2(base),
          animator{std::make_unique<friz::DisplaySyncController>(this)} {
        juce::ignoreUnused(uiBase);
        for (auto const s: {&slider1, &slider2}) {
            s->setSliderStyle(juce::Slider::Rotary);
            s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            s->setDoubleClickReturnValue(true, 0.0);
            s->setScrollWheelEnabled(true);
            s->setInterceptsMouseClicks(false, false);
        }
        slider1.setBufferedToImage(true);
        slider1.setLookAndFeel(&slider1LAF);
        slider2LAF.setEditable(showSlider2.load());
        slider2.setBufferedToImage(true);
        slider2.setLookAndFeel(&slider2LAF);

        addAndMakeVisible(slider1);
        addAndMakeVisible(slider2);

        label.setText(labelText, juce::dontSendNotification);
        label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        label2.setText(getDisplayValue(slider2), juce::dontSendNotification);

        labelLookAndFeel.setFontScale(FontHuge);
        labelLookAndFeel1.setFontScale(FontNormal);
        labelLookAndFeel1.setJustification(juce::Justification::centredBottom);
        labelLookAndFeel1.setAlpha(0.f);
        labelLookAndFeel2.setFontScale(FontNormal);
        labelLookAndFeel2.setJustification(juce::Justification::centredTop);
        labelLookAndFeel2.setAlpha(0.f);

        label.setLookAndFeel(&labelLookAndFeel);
        label1.setLookAndFeel(&labelLookAndFeel1);
        label2.setLookAndFeel(&labelLookAndFeel2);

        for (auto &l: {&label, &label1, &label2}) {
            l->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(l);
        }

        setInterceptsMouseClicks(true, false);
    }

    TwoValueRotarySlider::~TwoValueRotarySlider() {
        slider1.setLookAndFeel(nullptr);
        slider2.setLookAndFeel(nullptr);
        for (auto &l: {&label, &label1, &label2}) {
            l->setLookAndFeel(nullptr);
        }
    }

    juce::String TwoValueRotarySlider::getDisplayValue(juce::Slider &s) {
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

    void TwoValueRotarySlider::mouseUp(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseUp(event);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        } else {
            slider2.mouseUp(event);
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }

    void TwoValueRotarySlider::mouseDown(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDown(event);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        } else {
            slider2.mouseDown(event);
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }

    void TwoValueRotarySlider::mouseDrag(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDrag(event);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        } else {
            slider2.mouseDrag(event);
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }

    void TwoValueRotarySlider::mouseEnter(const juce::MouseEvent &event) {
        slider1.mouseEnter(event);
        slider2.mouseEnter(event);
        mouseOver.store(true);
        labelLookAndFeel.setAlpha(0.f);
        labelLookAndFeel1.setAlpha(1.f);
        labelLookAndFeel2.setAlpha(1.f);
        animator.cancelAnimation(animationId, false);
        repaint();
    }

    void TwoValueRotarySlider::mouseExit(const juce::MouseEvent &event) {
        slider1.mouseExit(event);
        slider2.mouseExit(event);
        mouseOver.store(false);

        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto effect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {1.5f}, {0.f}, 1000, friz::Parametric::kLinear)
        };
        effect->updateFn = [this](int, const auto &vals) {
            auto val = juce::jmin(vals[0], 1.0f);
            labelLookAndFeel.setAlpha(1 - val);
            labelLookAndFeel1.setAlpha(val);
            labelLookAndFeel2.setAlpha(val);
            for (auto &l:{&label, &label1, &label2}) {
                l->repaint();
            }
        };

        animator.addAnimation(std::move(effect));
    }

    void TwoValueRotarySlider::mouseMove(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
    }

    void TwoValueRotarySlider::mouseDoubleClick(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDoubleClick(event);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        } else {
            slider2.mouseDoubleClick(event);
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }

    void TwoValueRotarySlider::mouseWheelMove(const juce::MouseEvent &event,
                                              const juce::MouseWheelDetails &wheel) {
        if (!showSlider2.load() || !event.mods.isCommandDown()) {
            slider1.mouseWheelMove(event, wheel);
            label1.setText(getDisplayValue(slider1), juce::dontSendNotification);
        } else {
            slider2.mouseWheelMove(event, wheel);
            label2.setText(getDisplayValue(slider2), juce::dontSendNotification);
        }
    }


    void TwoValueRotarySlider::resized() {
        slider1.setBounds(getLocalBounds());
        slider2.setBounds(getLocalBounds());

        auto localBound = getLocalBounds().toFloat();
        auto labelBound = localBound.withSizeKeepingCentre(localBound.getWidth() * 0.7f,
                                                           localBound.getHeight() * 0.6f);
        label.setBounds(labelBound.toNearestInt());
        if (showSlider2.load()) {
            auto valueBound = labelBound.removeFromTop(labelBound.getHeight() * 0.5f);
            label1.setBounds(valueBound.toNearestInt());
            label2.setBounds(labelBound.toNearestInt());
        } else {
            label1.setBounds(getLocalBounds());
        }
    }
}
