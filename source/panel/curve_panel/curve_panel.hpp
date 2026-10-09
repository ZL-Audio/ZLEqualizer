// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "background_panel/background_panel.hpp"
#include "fft_panel/fft_panel.hpp"
#include "fft_panel/match_fft_panel.hpp"
#include "response_panel/response_panel.hpp"
#include "output_panel/output_panel.hpp"
#include "analyzer_panel/analyzer_panel.hpp"
#include "piano_roll_panel/piano_roll_panel.hpp"
#include "value_note_panel/value_note_panel.hpp"
#include "meter_panel/meter_panel.hpp"
#include "../top_panel/top_panel.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component,
                             private juce::ValueTree::Listener,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor& p, zlgui::UIBase& base,
                            multilingual::TooltipHelper& tooltip_helper);

        ~CurvePanel() override;

        void paintOverChildren(juce::Graphics& g) override;

        void run() override;

        void resized() override;

        void mouseDown(const juce::MouseEvent&) override;

        void repaintCallBack();

        void repaintCallBackSlow();

        void updateBand();

        void updateSampleRate(double sample_rate);

        void startThreads();

        void stopThreads();

        auto& getFFTPanel() {
            return fft_panel_;
        }

        auto& getOutputPanel() {
            return output_panel_;
        }

        auto& getMatchFFTPanel() {
            return match_fft_panel_;
        }

    private:
        struct ValueNoteState {
            bool show_values{false}, piano_visible{false}, mouse_over{false}, dragging{false};
            juce::Point<float> position;
            int note{-1};
            double frequency{0.};
            float gain{0.f}, max_db{0.f};

            bool operator==(const ValueNoteState& other) const {
                return show_values == other.show_values && piano_visible == other.piano_visible
                    && mouse_over == other.mouse_over && dragging == other.dragging
                    && position == other.position && note == other.note
                    && juce::exactlyEqual(frequency, other.frequency) && juce::exactlyEqual(gain, other.gain)
                    && juce::exactlyEqual(max_db, other.max_db);
            }
        };

        zlgui::UIBase& base_;
        std::atomic<float>& value_display_on_ref_;
        std::atomic<float>& meter_display_on_ref_;
        std::atomic<float>& eq_max_db_idx_ref_;
        BackgroundPanel background_panel_;
        FFTPanel fft_panel_;
        ResponsePanel response_panel_;
        MatchFFTPanel match_fft_panel_;
        ScalePanel scale_panel_;
        TopPanel top_panel_;
        OutputPanel output_panel_;
        AnalyzerPanel analyzer_panel_;
        PianoRollPanel piano_roll_panel_;
        ValueNotePanel value_note_panel_;
        MeterPanel meter_panel_;
        ValueNoteState value_note_state_;
        double fft_max_{0.}, frequency_max_{10.};
        bool piano_roll_visible_{false};
        std::atomic<bool> is_match_on_{false};

        void updatePianoRollVisibility();

        void updateValueNotePanel(bool force = false);

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
