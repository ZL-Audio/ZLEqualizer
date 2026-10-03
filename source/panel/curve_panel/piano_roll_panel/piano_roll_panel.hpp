// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "piano_keys_panel.hpp"
#include "piano_band_panel.hpp"
#include "piano_note_panel.hpp"
#include "piano_toggle_panel.hpp"

namespace zlpanel {
    class PianoRollPanel final : public juce::Component {
    public:
        explicit PianoRollPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~PianoRollPanel() override;

        void resized() override;

        bool hitTest(int x, int y) override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void mouseEnter(const juce::MouseEvent& event) override;

        void mouseMove(const juce::MouseEvent& event) override;

        void mouseExit(const juce::MouseEvent& event) override;

        void updateBand();

        void updateSampleRate(double sample_rate);

        void repaintCallBack();

        juce::Rectangle<float> getKeyBounds() const { return getLocalBounds().toFloat(); }

        bool isPianoRollVisible() const { return piano_roll_visible_; }

    private:
        struct BandState {
            juce::RangedAudioParameter* parameter{};
            std::atomic<float>* frequency{};
            std::atomic<float>* status{};
            float cached_frequency{-1.f};
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& piano_roll_on_ref_;
        PianoKeysPanel keys_panel_;
        PianoBandPanel bands_panel_;
        PianoNotePanel note_panel_;
        PianoTogglePanel toggle_panel_;
        std::array<BandState, zlp::kBandNum> bands_{};
        std::array<int, zlp::kBandNum> band_notes_{};
        size_t drag_band_{zlp::kBandNum};
        double slider_max_{0.};
        int first_note_{0}, last_note_{-1}, hovered_note_{-1};
        float drag_x_{0.f}, pointer_x_{0.f};
        bool piano_roll_visible_{false};

        void updateBands(bool force = false, bool geometry_changed = false);

        void updateHover(juce::Point<float> point);

        void updateNote();

        void endGesture();

        void lookAndFeelChanged() override;

        void visibilityChanged() override;
    };
}
