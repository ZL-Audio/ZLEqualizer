// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "dragger_component.hpp"

namespace zlgui {
    class DraggerParameterAttach final : private Dragger::Listener {
    public:
        DraggerParameterAttach(juce::RangedAudioParameter &parameter_x,
                               juce::NormalisableRange<float> n_range_x,
                               juce::RangedAudioParameter &parameter_y,
                               juce::NormalisableRange<float> n_range_y,
                               Dragger &dragger_c,
                               juce::UndoManager *undo_manager = nullptr);

        ~DraggerParameterAttach() override;

        void sendInitialUpdate();

        void enableX(const bool f) { is_x_attached_.store(f); }

        void enableY(const bool f) { is_y_attached_.store(f); }

        void setX(float new_value) const;

        void setY(float new_value) const;

    private:
        void draggerValueChanged(Dragger *dragger) override;

        void dragStarted(Dragger *dragger) override;

        void dragEnded(Dragger *dragger) override;

        Dragger &dragger_;
        juce::ParameterAttachment attachment_x_, attachment_y_;
        juce::NormalisableRange<float> range_x_, range_y_;
        std::atomic<bool> is_x_attached_{true}, is_y_attached_{true};
    };
} // zlgui
