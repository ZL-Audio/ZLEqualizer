// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlpanel {
    static constexpr float kButtonWidthP = 3.22f, kButtonHeightP = 3.17f;
    static constexpr float kBoxHeightP = 2.14f;
    static constexpr float kRotarySliderWidthP = 6.44f;

    inline void attach(const std::vector<juce::Button *> &buttons,
                       const std::vector<std::string> &ids,
                       juce::AudioProcessorValueTreeState &parameters,
                       juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> &attachments) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            attachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                    parameters, ids[i], *buttons[i]));
        }
    }

    inline void attach(const std::vector<juce::Button *> &buttons,
                       const std::vector<std::string> &ids,
                       juce::AudioProcessorValueTreeState &parameters,
                       juce::OwnedArray<zlgui::ButtonCusAttachment<true> > &attachments) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            attachments.add(
                std::make_unique<zlgui::ButtonCusAttachment<true> >(
                    parameters, ids[i], *buttons[i]));
        }
    }

    inline void attach(const std::vector<juce::Button *> &buttons,
                       const std::vector<std::string> &ids,
                       juce::AudioProcessorValueTreeState &parameters,
                       juce::OwnedArray<zlgui::ButtonCusAttachment<false> > &attachments) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            attachments.add(
                std::make_unique<zlgui::ButtonCusAttachment<false> >(
                    parameters, ids[i], *buttons[i]));
        }
    }

    inline void attach(const std::vector<juce::ComboBox *> &boxes,
                       const std::vector<std::string> &ids,
                       juce::AudioProcessorValueTreeState &parameters,
                       juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> &attachments) {
        for (size_t i = 0; i < boxes.size(); ++i) {
            attachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                    parameters, ids[i], *boxes[i]));
        }
    }

    inline void attach(const std::vector<juce::Slider *> &sliders,
                       const std::vector<std::string> &ids,
                       juce::AudioProcessorValueTreeState &parameters,
                       juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> &attachments) {
        for (size_t i = 0; i < sliders.size(); ++i) {
            attachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    parameters, ids[i], *sliders[i]));
        }
    }
}
