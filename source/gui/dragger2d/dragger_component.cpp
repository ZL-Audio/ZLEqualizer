// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_component.hpp"

namespace zlgui {
    Dragger::Dragger(UIBase &base)
        : ui_base_(base), dragger_laf_(base) {
        button_.addMouseListener(this, false);
        dragger_laf_.setColour(ui_base_.getColorMap1(1));
        button_.setClickingTogglesState(false);
        setInterceptsMouseClicks(false, true);
        button_.setLookAndFeel(&dragger_laf_);
        addAndMakeVisible(button_);
    }

    Dragger::~Dragger() {
        button_.removeMouseListener(this);
    }

    bool Dragger::updateButton(const juce::Point<float> &center) {
        if (std::abs(button_pos_.x - center.x) > 0.1f || std::abs(button_pos_.y - center.y) > 0.1f) {
            button_pos_ = center;
            button_.setTransform(juce::AffineTransform::translation(button_pos_.x, button_pos_.y));
            return true;
        }
        return false;
    }

    bool Dragger::updateButton() {
        return updateButton(current_pos_);
    }

    void Dragger::mouseDown(const juce::MouseEvent &e) {
        current_pos_ = button_pos_;
        global_pos_ = e.position + button_pos_;
        button_.setToggleState(true, juce::NotificationType::sendNotificationSync);
        const BailOutChecker checker(this);
        listeners_.callChecked(checker, [&](Dragger::Listener &l) { l.dragStarted(this); });
    }

    void Dragger::mouseUp(const juce::MouseEvent &e) {
        juce::ignoreUnused(e);
        const BailOutChecker checker(this);
        listeners_.callChecked(checker, [&](Dragger::Listener &l) { l.dragEnded(this); });
    }

    void Dragger::mouseDrag(const juce::MouseEvent &e) {
        // calculate shift and update global position
        const auto new_global_pos = e.position + button_pos_;
        auto shift = new_global_pos - global_pos_;
        const auto old_shift = shift;
        // apply sensitivity
        if (e.mods.isShiftDown()) {
            shift.setX(shift.getX() * ui_base_.getSensitivity(SensitivityIdx::kMouseDragFine));
            shift.setY(shift.getY() * ui_base_.getSensitivity(SensitivityIdx::kMouseDragFine));
        }
        if (e.mods.isCommandDown()) {
            if (e.mods.isLeftButtonDown()) {
                shift.setX(0.f);
            } else {
                shift.setY(0.f);
            }
        }
        // update current position
        const auto old_current_pos = current_pos_;
        if (check_center_) {
            current_pos_ = check_center_(current_pos_, current_pos_ + shift);
        } else {
            current_pos_ = current_pos_ + shift;
        }
        current_pos_ = button_area_.getConstrainedPoint(current_pos_);
        // shift global position accordingly
        const auto actual_shift = current_pos_ - old_current_pos;
        if (std::abs(shift.x) > 1e-10f) {
            global_pos_.x += actual_shift.x / shift.x * old_shift.x;
        }
        if (std::abs(shift.y) > 1e-10f) {
            global_pos_.y += actual_shift.y / shift.y * old_shift.y;
        }
        // update x/y portion
        x_portion_ = (current_pos_.getX() - button_area_.getX()) / button_area_.getWidth();
        y_portion_ = 1.f - (current_pos_.getY() - button_area_.getY()) / button_area_.getHeight();
        // call listeners
        const BailOutChecker checker(this);
        listeners_.callChecked(checker, [&](Listener &l) { l.draggerValueChanged(this); });
    }

    void Dragger::setButtonArea(const juce::Rectangle<float> bound) {
        button_area_ = bound;

        const auto radius = static_cast<int>(std::round(ui_base_.getFontSize() * scale_ * .5f));
        button_.setBounds(juce::Rectangle<int>(-radius, -radius, radius * 2, radius * 2));
        updateButton({-99999.f, -99999.f});

        auto laf_bound = button_.getBounds().toFloat().withPosition(0.f, 0.f);
        dragger_laf_.updatePaths(laf_bound);

        setXPortion(x_portion_);
        setYPortion(y_portion_);
    }

    void Dragger::addListener(Listener *listener) {
        listeners_.add(listener);
    }

    void Dragger::removeListener(Listener *listener) {
        listeners_.remove(listener);
    }

    void Dragger::setXPortion(const float x) {
        x_portion_ = x;
        current_pos_.x = button_area_.getX() + x * button_area_.getWidth();
    }

    void Dragger::setYPortion(const float y) {
        y_portion_ = y;
        current_pos_.y = button_area_.getY() + (1.f - y) * button_area_.getHeight();
    }

    float Dragger::getXPortion() const {
        return x_portion_;
    }

    float Dragger::getYPortion() const {
        return y_portion_;
    }

    void Dragger::visibilityChanged() {
        updateButton({-100000.f, -100000.f});
    }
} // zlgui
