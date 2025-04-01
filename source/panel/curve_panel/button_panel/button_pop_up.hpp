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

namespace zlPanel {
    class ButtonPopUp final : public juce::Component,
                              public juce::ComponentListener {
    private:
        class PitchLabel final : public juce::Component, private juce::Label::Listener {
        public:
            explicit PitchLabel(zlInterface::UIBase &base, juce::RangedAudioParameter *freq);

            void setFreq(double freq);

            void resized() override {
                label.setBounds(getLocalBounds());
            }

        private:
            zlInterface::UIBase &uiBase;
            juce::RangedAudioParameter *freqPara;
            zlInterface::NameLookAndFeel laf;
            juce::Label label;

            static constexpr std::array pitchLookUp{
                "A", "A#", "B", "C",
                "C#", "D", "D#", "E",
                "F", "F#", "G", "G#"
            };


            void editorShown(juce::Label *, juce::TextEditor &editor) override;

            void editorHidden(juce::Label *, juce::TextEditor &) override;

            void labelTextChanged(juce::Label *labelThatHasChanged) override {
                juce::ignoreUnused(labelThatHasChanged);
            }
        };

    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUp() override;

        void resized() override;

        void updateBounds(const juce::Component &component);

        void visibilityChanged() override {
            if (isVisible()) {
                setBounds(previousBound);
            }
        }

    private:
        static constexpr float widthP{7.7916666f}, heightP{4.16667f};

        size_t band;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<float> &fType, &freqPara;
        float direction = -1.f;
        juce::Rectangle<int> previousBound{};

        ButtonPopUpBackground background;

        PitchLabel pitchLabel;

        void updateLabel();
    };
} // zlPanel
