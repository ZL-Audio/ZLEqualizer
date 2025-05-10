// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../label/name_look_and_feel.hpp"
#include "compact_linear_slider_look_and_feel.hpp"
#include "../extra_slider/snapping_slider.h"

namespace zlgui {
    class CompactLinearSlider : public juce::Component,
                                private juce::Label::Listener,
                                private juce::Slider::Listener,
                                public juce::SettableTooltipClient {
    public:
        explicit CompactLinearSlider(const juce::String &label_text, UIBase &base,
                                     multilingual::Labels label_idx = multilingual::Labels::kLabelNum);

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

        inline juce::Slider &getSlider() { return slider_; }

        inline void setEditable(const bool x) {
            name_look_and_feel_.setEditable(x);
            text_look_and_feel_.setEditable(x);
            setInterceptsMouseClicks(x, false);
            repaint();
        }

        inline void setPadding(const float lr, const float ub) {
            lr_pad_ = lr;
            ub_pad_ = ub;
        }

        void updateDisplayValue() {
            text_.setText(getDisplayValue(slider_), juce::dontSendNotification);
            text_.repaint();
        }

    private:
        UIBase &ui_base_;

        CompactLinearSliderLookAndFeel slider_look_and_feel_;
        NameLookAndFeel name_look_and_feel_, text_look_and_feel_;
        SnappingSlider slider_;
        juce::Label label_, text_;

        float lr_pad_ = 0, ub_pad_ = 0;

        juce::String getDisplayValue(juce::Slider &s);

        void labelTextChanged(juce::Label *label_that_has_changed) override;

        void editorShown(juce::Label *l, juce::TextEditor &editor) override;

        void editorHidden(juce::Label *l, juce::TextEditor &editor) override;

        void sliderValueChanged(juce::Slider *slider) override;
    };
}
