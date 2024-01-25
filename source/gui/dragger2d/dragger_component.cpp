// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_component.hpp"

namespace zlInterface {
    Dragger::Dragger(UIBase &base)
        : uiBase(base), draggerLAF(base) {
        button.addMouseListener(this, false);
        draggerLAF.setColour(uiBase.getColorMap1(1));
        button.setClickingTogglesState(false);
        button.setLookAndFeel(&draggerLAF);
        addAndMakeVisible(button);
    }

    Dragger::~Dragger() {
        button.removeMouseListener(this);
        button.setLookAndFeel(nullptr);
    }

    void Dragger::mouseDown(const juce::MouseEvent &event) {
        isSelected.store(true);
        button.setToggleState(true, juce::NotificationType::sendNotificationSync);
        dragger.startDraggingComponent(&button, event);

        const BailOutChecker checker(this);
        listeners.callChecked(checker, [&](Dragger::Listener &l) { l.dragStarted(this); });
    }

    void Dragger::mouseUp(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        isSelected.store(false);
        const BailOutChecker checker(this);
        listeners.callChecked(checker, [&](Dragger::Listener &l) { l.dragEnded(this); });
    }

    void Dragger::mouseDrag(const juce::MouseEvent &event) {
        if (isSelected.load()) {
            dragger.dragComponent(&button, event, &constrainer);
            const auto buttonBound = button.getBoundsInParent().toFloat();
            xPortion.store((buttonBound.getCentreX() - buttonArea.getX()) / buttonArea.getWidth());
            yPortion.store((buttonArea.getBottom() - buttonBound.getCentreY()) / buttonArea.getHeight());

            const BailOutChecker checker(this);
            listeners.callChecked(checker, [&](Listener &l) { l.draggerValueChanged(this); });
        }
    }

    void Dragger::resized() {
        const auto bound = getLocalBounds().toFloat();
        buttonArea = juce::Rectangle<float>(lPadding, uPadding,
                                            bound.getWidth() - lPadding - rPadding,
                                            bound.getHeight() - uPadding - bPadding);
        auto buttonBound = juce::Rectangle<float>(uiBase.getFontSize() * scale.load(),
                                                  uiBase.getFontSize() * scale.load());
        buttonBound.setCentre(buttonArea.getX() + xPortion.load() * buttonArea.getWidth(),
                              buttonArea.getBottom() - yPortion.load() * buttonArea.getHeight());
        button.setBounds(buttonBound.toNearestInt());
        // set constrainer
        const auto minimumOffset = uiBase.getFontSize() * scale.load() * .5f;
        constrainer.setMinimumOnscreenAmounts(static_cast<int>(std::floor(minimumOffset + uPadding + .5f)),
                                              static_cast<int>(std::floor(minimumOffset + lPadding + .5f)),
                                              static_cast<int>(std::floor(minimumOffset + bPadding + .5f)),
                                              static_cast<int>(std::floor(minimumOffset + rPadding + .5f)));
    }

    void Dragger::addListener(Listener *listener) {
        listeners.add(listener);
    }

    void Dragger::removeListener(Listener *listener) {
        listeners.remove(listener);
    }

    void Dragger::setXPortion(const float x) {
        xPortion.store(x);
        auto buttonBound = button.getBoundsInParent().toFloat();
        buttonBound.setCentre(buttonArea.getX() + x * buttonArea.getWidth(),
                              buttonBound.getCentreY());
        button.setBounds(buttonBound.toNearestInt());
    }

    void Dragger::setYPortion(const float y) {
        yPortion.store(y);
        auto buttonBound = button.getBoundsInParent().toFloat();
        buttonBound.setCentre(buttonBound.getCentreX(),
                              buttonArea.getBottom() - y * buttonArea.getHeight());
        button.setBounds(buttonBound.toNearestInt());
    }

    float Dragger::getXPortion() const {
        return xPortion.load();
    }

    float Dragger::getYPortion() const {
        return yPortion.load();
    }
} // zlInterface
