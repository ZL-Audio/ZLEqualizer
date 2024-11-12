// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_combobox.hpp"

namespace zlInterface {
    CompactCombobox::CompactCombobox(const juce::String &labelText, const juce::StringArray &choices,
                                     UIBase &base) : uiBase(base), boxLookAndFeel(base),
                                                     animator{} {
        juce::ignoreUnused(labelText);
        comboBox.addItemList(choices, 1);
        comboBox.setScrollWheelEnabled(false);
        comboBox.setInterceptsMouseClicks(false, false);
        comboBox.setLookAndFeel(&boxLookAndFeel);
        addAndMakeVisible(comboBox);

        setEditable(true);
    }


    CompactCombobox::~CompactCombobox() {
        animator.cancelAllAnimations(false);
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
        auto frizEffect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {boxLookAndFeel.getBoxAlpha()}, {1.f}, 1000, friz::Parametric::kEaseInQuad)
        };
        frizEffect->updateFn = [this](int, const auto &vals) {
            boxLookAndFeel.setBoxAlpha(vals[0]);
            comboBox.repaint();
        };
        animator.addAnimation(std::move(frizEffect));
    }

    void CompactCombobox::mouseExit(const juce::MouseEvent &event) {
        comboBox.mouseExit(event);
        animator.cancelAnimation(animationId, false);
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto frizEffect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {boxLookAndFeel.getBoxAlpha()}, {0.f}, 1000, friz::Parametric::kEaseOutQuad)
        };
        frizEffect->updateFn = [this](int, const auto &vals) {
            boxLookAndFeel.setBoxAlpha(vals[0]);
            comboBox.repaint();
        };
        animator.addAnimation(std::move(frizEffect));
    }

    void CompactCombobox::mouseMove(const juce::MouseEvent &event) {
        comboBox.mouseMove(event);
    }
}
