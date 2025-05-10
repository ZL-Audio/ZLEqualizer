// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../panel_definitons.hpp"
#include "../../helper/helper.hpp"
#include "button_pop_up_background.hpp"

namespace zlpanel {
    class ButtonPopUp final : public juce::Component,
                              public juce::ComponentListener {
    private:
        class PitchLabel : public juce::Component,
                           private juce::Label::Listener,
                           private juce::TextEditor::Listener {
        public:
            explicit PitchLabel(zlgui::UIBase &base, juce::RangedAudioParameter *freq);

            void setFreq(double freq);

            void resized() override {
                label_.setBounds(getLocalBounds());
            }

        private:
            zlgui::UIBase &ui_base_;
            juce::RangedAudioParameter *freq_para_;
            zlgui::NameLookAndFeel laf_;
            juce::Label label_;
            bool has_editor_changed_{false};

            static constexpr std::array kPitchLookUp{
                "A", "A#", "B", "C",
                "C#", "D", "D#", "E",
                "F", "F#", "G", "G#"
            };

            void editorShown(juce::Label *, juce::TextEditor &editor) override;

            void editorHidden(juce::Label *, juce::TextEditor &) override;

            void labelTextChanged(juce::Label *) override {}

            void textEditorTextChanged(juce::TextEditor &) override;
        };

    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA,
                             zlgui::UIBase &base);

        ~ButtonPopUp() override;

        void resized() override;

        void updateBounds(const juce::Component &component);

        void visibilityChanged() override {
            if (isVisible()) {
                setBounds(previous_bound_);
            }
        }

    private:
        static constexpr float kWidthP{7.7916666f}, kHeightP{4.16667f};

        size_t band_idx_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        std::atomic<float> &ftype_, &freq_para_;
        float direction_ = -1.f;
        juce::Rectangle<int> previous_bound_{};

        ButtonPopUpBackground background_;

        PitchLabel pitch_label_;

        void updateLabel();
    };
} // zlpanel
