// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

#include "../../../gui/gui.hpp"

namespace zlpanel {
    class PianoKeysPanel final : public juce::Component {
    public:
        struct Key {
            juce::Rectangle<float> bounds;
            juce::RectangleList<float> region;
        };

        explicit PianoKeysPanel(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateSampleRate(double sample_rate);

        const Key* getKey(int note) const;

        int getNoteAt(juce::Point<float> point) const;

        float noteToX(double note) const;

        double xToNote(float x) const;

    private:
        zlgui::UIBase& base_;
        double fft_max_{0.};
        float a4_x_{0.f}, note_width_{0.f};
        int first_note_{0};
        std::vector<Key> keys_;

        juce::Rectangle<float> getBlackKeyBounds(int note) const;

        juce::Rectangle<float> getWhiteKeyBounds(int note) const;

        void updateGeometry();

        void lookAndFeelChanged() override;
    };
}
