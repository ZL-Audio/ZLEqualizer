//
// Created by Zishu Liu on 12/30/23.
//

#include "compact_linear_slider.hpp"

namespace zlInterface {
    CompactLinearSlider::CompactLinearSlider(const juce::String &labelText, UIBase &base) : uiBase(base),
        sliderLookAndFeel(base), nameLookAndFeel(base), textLookAndFeel(base),
        animator{std::make_unique<friz::DisplaySyncController>(this)} {
        juce::ignoreUnused(uiBase);

        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        sliderLookAndFeel.setTextAlpha(0.f);
        slider.setTextBoxIsEditable(false);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        slider.setDoubleClickReturnValue(true, 0.0);
        slider.setScrollWheelEnabled(true);
        slider.setInterceptsMouseClicks(false, false);
        slider.setBufferedToImage(true);
        slider.setLookAndFeel(&sliderLookAndFeel);
        addAndMakeVisible(slider);

        text.setText(getDisplayValue(slider), juce::dontSendNotification);
        textLookAndFeel.setAlpha(0.f);
        textLookAndFeel.setFontScale(FontLarge);
        text.setLookAndFeel(&textLookAndFeel);
        text.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(text);

        // setup label
        label.setText(labelText, juce::dontSendNotification);
        label.setLookAndFeel(&nameLookAndFeel);
        nameLookAndFeel.setFontScale(FontLarge);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        setEditable(true);
    }

    CompactLinearSlider::~CompactLinearSlider() {
        slider.setLookAndFeel(nullptr);
        label.setLookAndFeel(nullptr);
        text.setLookAndFeel(nullptr);
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
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto effect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {1.5f}, {0.f}, 1000, friz::Parametric::kLinear)
        };
        effect->updateFn = [this](int, const auto &vals) {
            auto val = juce::jmin(vals[0], 1.0f);
            textLookAndFeel.setAlpha(val);
            nameLookAndFeel.setAlpha(1.f - val);
            text.repaint();
            label.repaint();
        };
        animator.addAnimation(std::move(effect));
    }

    void CompactLinearSlider::mouseMove(const juce::MouseEvent &event) {
        slider.mouseMove(event);
    }

    void CompactLinearSlider::mouseDoubleClick(const juce::MouseEvent &event) {
        slider.mouseDoubleClick(event);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
    }

    void CompactLinearSlider::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        slider.mouseWheelMove(event, wheel);
        text.setText(getDisplayValue(slider), juce::dontSendNotification);
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
