// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLTest_DRAGGER_PARAMETER_ATTACH_HPP
#define ZLTest_DRAGGER_PARAMETER_ATTACH_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "dragger_component.hpp"

namespace zlInterface {
    class DraggerParameterAttach final : private Dragger::Listener {
    public:
        DraggerParameterAttach(juce::RangedAudioParameter &parameterX,
                               juce::NormalisableRange<float> nRangeX,
                               juce::RangedAudioParameter &parameterY,
                               juce::NormalisableRange<float> nRangeY,
                               Dragger &draggerC,
                               juce::UndoManager *undoManager = nullptr);

        ~DraggerParameterAttach() override;

        void sendInitialUpdate();

        void enableX(const bool f) { isXAttached.store(f); }

        void enableY(const bool f) { isYAttached.store(f); }

        void setX(float newValue) const;

        void setY(float newValue) const;

    private:
        void draggerValueChanged(Dragger *dragger) override;

        void dragStarted(Dragger *dragger) override;

        void dragEnded(Dragger *dragger) override;

        Dragger &dragger;
        juce::ParameterAttachment attachmentX, attachmentY;
        juce::NormalisableRange<float> rangeX, rangeY;
        std::atomic<bool> isXAttached{true}, isYAttached{true};
    };
} // zlInterface

#endif //ZLTest_DRAGGER_PARAMETER_ATTACH_HPP
