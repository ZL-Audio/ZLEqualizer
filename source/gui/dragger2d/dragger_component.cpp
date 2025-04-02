// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_component.hpp"

namespace zlInterface {
    Dragger::Dragger(UIBase &base)
        : uiBase(base), draggerLAF(base) {
        button.addMouseListener(this, false);
        draggerLAF.setColour(uiBase.getColorMap1(1));
        button.setClickingTogglesState(false);
        setInterceptsMouseClicks(false, true);
        button.setLookAndFeel(&draggerLAF);
        addAndMakeVisible(button);
    }

    Dragger::~Dragger() {
        button.removeMouseListener(this);
    }

    bool Dragger::updateButton(const juce::Point<float> &center) {
        if (std::abs(buttonPos.x - center.x) > 0.1f || std::abs(buttonPos.y - center.y) > 0.1f) {
            buttonPos = center;
            button.setTransform(juce::AffineTransform::translation(buttonPos.x, buttonPos.y));
            return true;
        }
        return false;
    }

    bool Dragger::updateButton() {
        return updateButton(currentPos);
    }

    void Dragger::mouseDown(const juce::MouseEvent &e) {
        currentPos = buttonPos;
        globalPos = e.position + buttonPos;
        button.setToggleState(true, juce::NotificationType::sendNotificationSync);
        const BailOutChecker checker(this);
        listeners.callChecked(checker, [&](Dragger::Listener &l) { l.dragStarted(this); });
    }

    void Dragger::mouseUp(const juce::MouseEvent &e) {
        juce::ignoreUnused(e);
        const BailOutChecker checker(this);
        listeners.callChecked(checker, [&](Dragger::Listener &l) { l.dragEnded(this); });
    }

    void Dragger::mouseDrag(const juce::MouseEvent &e) {
        // calculate shift and update global position
        const auto newGlobalPos = e.position + buttonPos;
        auto shift = newGlobalPos - globalPos;
        globalPos = newGlobalPos;
        // apply sensitivity
        if (e.mods.isShiftDown()) {
            shift.setX(shift.getX() * uiBase.getSensitivity(sensitivityIdx::mouseDragFine));
            shift.setY(shift.getY() * uiBase.getSensitivity(sensitivityIdx::mouseDragFine));
        }
        if (e.mods.isCommandDown()) {
            if (e.mods.isLeftButtonDown()) {
                shift.setX(0.f);
            } else {
                shift.setY(0.f);
            }
        }
        // update current position
        if (checkCenter) {
            currentPos = checkCenter(currentPos, currentPos + shift);
        } else {
            currentPos = currentPos + shift;
        }
        currentPos = buttonArea.getConstrainedPoint(currentPos);
        // update x/y portion
        xPortion = (currentPos.getX() - buttonArea.getX()) / buttonArea.getWidth();
        yPortion = 1.f - (currentPos.getY() - buttonArea.getY()) / buttonArea.getHeight();
        // call listeners
        const BailOutChecker checker(this);
        listeners.callChecked(checker, [&](Listener &l) { l.draggerValueChanged(this); });
    }

    void Dragger::setButtonArea(const juce::Rectangle<float> bound) {
        buttonArea = bound;

        const auto radius = static_cast<int>(std::round(uiBase.getFontSize() * scale * .5f));
        button.setBounds(juce::Rectangle<int>(-radius, -radius, radius * 2, radius * 2));
        updateButton({-99999.f, -99999.f});

        auto lafBound = button.getBounds().toFloat().withPosition(0.f, 0.f);
        draggerLAF.updatePaths(lafBound);

        setXPortion(xPortion);
        setYPortion(yPortion);
    }

    void Dragger::addListener(Listener *listener) {
        listeners.add(listener);
    }

    void Dragger::removeListener(Listener *listener) {
        listeners.remove(listener);
    }

    void Dragger::setXPortion(const float x) {
        xPortion = x;
        currentPos.x = buttonArea.getX() + x * buttonArea.getWidth();
    }

    void Dragger::setYPortion(const float y) {
        yPortion = y;
        currentPos.y = buttonArea.getY() + (1.f - y) * buttonArea.getHeight();
    }

    float Dragger::getXPortion() const {
        return xPortion;
    }

    float Dragger::getYPortion() const {
        return yPortion;
    }

    void Dragger::visibilityChanged() {
        updateButton({-100000.f, -100000.f});
    }
} // zlInterface
