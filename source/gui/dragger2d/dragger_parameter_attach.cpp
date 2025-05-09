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
    DraggerParameterAttach::DraggerParameterAttach(juce::RangedAudioParameter &parameterX,
                                                   juce::NormalisableRange<float> nRangeX,
                                                   juce::RangedAudioParameter &parameterY,
                                                   juce::NormalisableRange<float> nRangeY,
                                                   Dragger &draggerC,
                                                   juce::UndoManager *undoManager)
        : dragger(draggerC),
          attachmentX(parameterX, [this](const float f) { setX(f); }, undoManager),
          attachmentY(parameterY, [this](const float f) { setY(f); }, undoManager),
          rangeX(std::move(nRangeX)), rangeY(std::move(nRangeY)) {
        dragger.addListener(this);
    }

    DraggerParameterAttach::~DraggerParameterAttach() {
        dragger.removeListener(this);
    }

    void DraggerParameterAttach::sendInitialUpdate() {
        attachmentX.sendInitialUpdate();
        attachmentY.sendInitialUpdate();
    }

    void DraggerParameterAttach::setX(const float newValue) const {
        dragger.setXPortion(rangeX.convertTo0to1(rangeX.snapToLegalValue(newValue)));
    }

    void DraggerParameterAttach::setY(const float newValue) const {
        dragger.setYPortion(rangeY.convertTo0to1(rangeY.snapToLegalValue(newValue)));
    }

    void DraggerParameterAttach::dragStarted(Dragger *) {
        attachmentX.beginGesture();
        attachmentY.beginGesture();
    }

    void DraggerParameterAttach::dragEnded(Dragger *) {
        attachmentX.endGesture();
        attachmentY.endGesture();
    }

    void DraggerParameterAttach::draggerValueChanged(Dragger *) {
        if (isXAttached.load()) attachmentX.setValueAsPartOfGesture(rangeX.convertFrom0to1(
            std::clamp(dragger.getXPortion(), 0.f, 1.f)));
        if (isYAttached.load()) attachmentY.setValueAsPartOfGesture(rangeY.convertFrom0to1(
            std::clamp(dragger.getYPortion(), 0.f, 1.f)));
    }
} // zlgui
