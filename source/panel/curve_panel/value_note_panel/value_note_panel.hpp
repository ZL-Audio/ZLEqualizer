// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../gui/gui.hpp"

namespace zlpanel {
    class ValueNotePanel final : public juce::Component {
    public:
        enum class Mode { kNote, kFrequencyGain, kFrequencyNote };

        explicit ValueNotePanel(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void setValues(Mode mode, bool piano_visible, double frequency, float gain, int note);

    private:
        zlgui::UIBase& base_;
        Mode mode_{Mode::kNote};
        bool piano_visible_{false};
        int frequency_tenths_{-1}, gain_hundredths_{0};
        int note_{-1};
        juce::String first_text_, second_text_;
        juce::ColourGradient gradient_;

        void lookAndFeelChanged() override;
    };
}
