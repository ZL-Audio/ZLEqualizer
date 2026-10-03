// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "piano_note_panel.hpp"
#include "../../helper/freq_note.hpp"

namespace zlpanel {
    PianoNotePanel::PianoNotePanel(zlgui::UIBase& base) : base_(base) {
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void PianoNotePanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        const auto background = base_.getBackgroundColour();
        g.setGradientFill({background.withAlpha(0.f), 0.f, 0.f,
                           background, bound.getWidth(), 0.f, false});
        g.fillRect(bound);
        g.setColour(base_.getTextColour());
        g.setFont(juce::FontOptions(base_.getFontSize() * 1.25f));
        g.drawText(text_, bound.reduced(base_.getFontSize() * .2f, 0.f), juce::Justification::centredRight);
    }

    void PianoNotePanel::setNote(const int note) {
        if (note_ != note) {
            note_ = note;
            text_ = note >= 0
                ? juce::String(freq_note::kNoteNames[static_cast<size_t>(note % 12)]) + juce::String(note / 12 - 1)
                : juce::String{};
            repaint();
        }
    }

    void PianoNotePanel::lookAndFeelChanged() {
        repaint();
    }
}
