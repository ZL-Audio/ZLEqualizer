// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "value_note_panel.hpp"
#include "../../helper/freq_note.hpp"

namespace zlpanel {
    ValueNotePanel::ValueNotePanel(zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void ValueNotePanel::paint(juce::Graphics& g) {
        auto first_bound = getLocalBounds().toFloat();
        const auto second_bound = first_bound.removeFromBottom(first_bound.getHeight() * .5f);
        if (mode_ == Mode::kNote) {
            first_bound = second_bound;
        }
        g.setGradientFill(gradient_);
        g.fillRect(second_bound);
        g.setColour(base_.getTextColour());
        g.setFont(juce::FontOptions(base_.getFontSize() * 1.25f));
        const auto padding = base_.getFontSize() * .2f;
        g.drawText(first_text_, first_bound.reduced(padding, 0.f), juce::Justification::centredRight);
        if (mode_ != Mode::kNote) {
            g.drawText(second_text_, second_bound.reduced(padding, 0.f), juce::Justification::centredRight);
        }
    }

    void ValueNotePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto second_bound = bound.removeFromBottom(bound.getHeight() * .5f);
        gradient_ = juce::ColourGradient{juce::Colours::transparentBlack, 0.f, second_bound.getY(),
                                         base_.getBackgroundColour(), second_bound.getRight(), second_bound.getY(),
                                         false};
    }

    void ValueNotePanel::setValues(const Mode mode, const bool piano_visible,
                                   const double frequency, const float gain, const int note) {
        const auto frequency_tenths = mode == Mode::kNote ? -1 : static_cast<int>(std::round(frequency * 10.0));
        const auto gain_hundredths = mode == Mode::kFrequencyGain ? static_cast<int>(std::round(gain * 100.f)) : 0;
        const auto display_note = mode == Mode::kFrequencyGain ? -1 : note;
        if (mode_ == mode && piano_visible_ == piano_visible && frequency_tenths_ == frequency_tenths
            && gain_hundredths_ == gain_hundredths && note_ == display_note) {
            return;
        }
        mode_ = mode;
        piano_visible_ = piano_visible;
        frequency_tenths_ = frequency_tenths;
        gain_hundredths_ = gain_hundredths;
        note_ = display_note;
        const auto note_text = note_ >= 0
            ? juce::String(freq_note::kNoteNames[static_cast<size_t>(note_ % 12)]) + juce::String(note_ / 12 - 1)
            : juce::String{};
        if (mode == Mode::kNote) {
            first_text_ = note_text;
            second_text_ = juce::String{};
        } else {
            if (frequency_tenths_ * 0.1 < 1000.) {
                first_text_ = juce::String(frequency_tenths_ * .1, 1);
            } else if (frequency_tenths_ * 0.1 < 10000.) {
                first_text_ = juce::String(static_cast<int>(std::round(frequency_tenths_ * .1)));
            } else {
                first_text_ = juce::String(frequency_tenths_ * .0001, 1) + "K";
            }
            if (mode == Mode::kFrequencyNote) {
                second_text_ = note_text;
            } else {
                second_text_ = juce::String(gain_hundredths_ * .01, 2);
            }
        }
        repaint();
    }

    void ValueNotePanel::lookAndFeelChanged() {
        repaint();
    }
}
