// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_parameter_attach.hpp"

namespace zlgui {
    DraggerParameterAttach::DraggerParameterAttach(juce::RangedAudioParameter &parameter_x,
                                                   juce::NormalisableRange<float> n_range_x,
                                                   juce::RangedAudioParameter &parameter_y,
                                                   juce::NormalisableRange<float> n_range_y,
                                                   Dragger &dragger_c,
                                                   juce::UndoManager *undo_manager)
        : dragger_(dragger_c),
          attachment_x_(parameter_x, [this](const float f) { setX(f); }, undo_manager),
          attachment_y_(parameter_y, [this](const float f) { setY(f); }, undo_manager),
          range_x_(std::move(n_range_x)), range_y_(std::move(n_range_y)) {
        dragger_.addListener(this);
    }

    DraggerParameterAttach::~DraggerParameterAttach() {
        dragger_.removeListener(this);
    }

    void DraggerParameterAttach::sendInitialUpdate() {
        attachment_x_.sendInitialUpdate();
        attachment_y_.sendInitialUpdate();
    }

    void DraggerParameterAttach::setX(const float new_value) const {
        dragger_.setXPortion(range_x_.convertTo0to1(range_x_.snapToLegalValue(new_value)));
    }

    void DraggerParameterAttach::setY(const float new_value) const {
        dragger_.setYPortion(range_y_.convertTo0to1(range_y_.snapToLegalValue(new_value)));
    }

    void DraggerParameterAttach::dragStarted(Dragger *) {
        attachment_x_.beginGesture();
        attachment_y_.beginGesture();
    }

    void DraggerParameterAttach::dragEnded(Dragger *) {
        attachment_x_.endGesture();
        attachment_y_.endGesture();
    }

    void DraggerParameterAttach::draggerValueChanged(Dragger *) {
        if (is_x_attached_.load()) attachment_x_.setValueAsPartOfGesture(range_x_.convertFrom0to1(
            std::clamp(dragger_.getXPortion(), 0.f, 1.f)));
        if (is_y_attached_.load()) attachment_y_.setValueAsPartOfGesture(range_y_.convertFrom0to1(
            std::clamp(dragger_.getYPortion(), 0.f, 1.f)));
    }
} // zlgui
