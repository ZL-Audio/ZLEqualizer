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
#include "button_pop_up_background.hpp"

namespace zlPanel {
    class ButtonPopUp final : public juce::Component,
                              public juce::ComponentListener {
    private:
        class PitchLabel final : public juce::Component {
        public:
            explicit PitchLabel(zlInterface::UIBase &base) : uiBase(base) {
            }

            void paint(juce::Graphics &g) override {
                g.setFont(uiBase.getFontSize() * 1.2f);
                g.setColour(uiBase.getTextColor());
                g.drawText(label, getLocalBounds(), juce::Justification::centredRight, false);
            }

            void setText(const juce::String &x) { label = x; }

        private:
            zlInterface::UIBase &uiBase;
            juce::String label;
        };

    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUp() override;

        void resized() override;

        void updateBounds(const juce::Component &component);

    private:
        static constexpr float widthP{7.7916666f}, heightP{4.16667f};

        size_t band;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<float> &fType, &freqPara;
        float direction = -1.f;
        juce::Rectangle<int> previousBound{}, previousCompBound{};

        ButtonPopUpBackground background;

        PitchLabel pitchLabel;

        static constexpr std::array pitchLookUp{
            "A", "A#", "B", "C",
            "C#", "D", "D#", "E",
            "F", "F#", "G", "G#"
        };

        void updateLabel();
    };
} // zlPanel
