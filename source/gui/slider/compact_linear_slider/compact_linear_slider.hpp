// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <friz/friz.h>

#include "../../label/name_look_and_feel.hpp"
#include "compact_linear_slider_look_and_feel.hpp"
#include "../extra_slider/snapping_slider.h"

namespace zlInterface {
    class CompactLinearSlider : public juce::Component,
                                private juce::Label::Listener,
                                private juce::Slider::Listener,
                                public juce::SettableTooltipClient {
    public:
        explicit CompactLinearSlider(const juce::String &labelText, UIBase &base,
                                     multilingual::labels labelIdx = multilingual::labels::labelNum);

        ~CompactLinearSlider() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        inline juce::Slider &getSlider() { return slider; }

        inline void setEditable(const bool x) {
            nameLookAndFeel.setEditable(x);
            textLookAndFeel.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline void setPadding(const float lr, const float ub) {
            lrPad.store(lr);
            ubPad.store(ub);
        }

        void updateDisplayValue() {
            text.setText(getDisplayValue(slider), juce::dontSendNotification);
            text.repaint();
        }

    private:
        UIBase &uiBase;

        CompactLinearSliderLookAndFeel sliderLookAndFeel;
        NameLookAndFeel nameLookAndFeel, textLookAndFeel;
        SnappingSlider slider;
        juce::Label label, text;

        friz::Animator animator{};
        static constexpr int animationId = 1;

        std::atomic<float> lrPad = 0, ubPad = 0;

        juce::String getDisplayValue(juce::Slider &s);

        void labelTextChanged(juce::Label *labelThatHasChanged) override;

        void editorShown(juce::Label *l, juce::TextEditor &editor) override;

        void editorHidden(juce::Label *l, juce::TextEditor &editor) override;

        void sliderValueChanged(juce::Slider *slider) override;

        void leaveAnimation();
    };
}
