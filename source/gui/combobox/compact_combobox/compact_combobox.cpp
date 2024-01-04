//
// Created by Zishu Liu on 12/30/23.
//

#include "compact_combobox.hpp"

namespace zlInterface {
    CompactCombobox::CompactCombobox(const juce::String &labelText, const juce::StringArray &choices,
                                     UIBase &base) : uiBase(base), boxLookAndFeel(base),
                                                     animator{std::make_unique<friz::DisplaySyncController>(this)} {
        juce::ignoreUnused(labelText);
        comboBox.addItemList(choices, 1);
        comboBox.setScrollWheelEnabled(false);
        comboBox.setInterceptsMouseClicks(false, false);
        comboBox.setBufferedToImage(true);
        comboBox.setLookAndFeel(&boxLookAndFeel);
        addAndMakeVisible(comboBox);

        setEditable(true);
    }


    CompactCombobox::~CompactCombobox() {
        comboBox.setLookAndFeel(nullptr);
    }

    void CompactCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), juce::jmin(bound.getHeight(),
                                                                         uiBase.getFontSize() * 2.f));
        comboBox.setBounds(bound.toNearestInt());
    }

    void CompactCombobox::mouseUp(const juce::MouseEvent &event) {
        comboBox.mouseUp(event);
    }

    void CompactCombobox::mouseDown(const juce::MouseEvent &event) {
        comboBox.mouseDown(event);
    }

    void CompactCombobox::mouseDrag(const juce::MouseEvent &event) {
        comboBox.mouseDrag(event);
    }

    void CompactCombobox::mouseEnter(const juce::MouseEvent &event) {
        comboBox.mouseEnter(event);
        animator.cancelAnimation(animationId, false);
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto effect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {boxLookAndFeel.getBoxAlpha()}, {1.f}, 1000, friz::Parametric::kEaseInQuad)
        };
        effect->updateFn = [this](int, const auto &vals) {
            boxLookAndFeel.setBoxAlpha(vals[0]);
            comboBox.repaint();
        };
        animator.addAnimation(std::move(effect));
    }

    void CompactCombobox::mouseExit(const juce::MouseEvent &event) {
        comboBox.mouseExit(event);
        if (!comboBox.isPopupActive()) {
            animator.cancelAnimation(animationId, false);
            if (animator.getAnimation(animationId) != nullptr)
                return;
            auto effect{
                friz::makeAnimation<friz::Parametric, 1>(
                    animationId, {boxLookAndFeel.getBoxAlpha()}, {0.f}, 1000, friz::Parametric::kEaseOutQuad)
            };
            effect->updateFn = [this](int, const auto &vals) {
                boxLookAndFeel.setBoxAlpha(vals[0]);
                comboBox.repaint();
            };
            animator.addAnimation(std::move(effect));
        }
    }

    void CompactCombobox::mouseMove(const juce::MouseEvent &event) {
        comboBox.mouseMove(event);
    }
}
