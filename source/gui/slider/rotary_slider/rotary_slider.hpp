// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_ROTARY_SLIDER_COMPONENT_H
#define ZL_ROTARY_SLIDER_COMPONENT_H

#include "../../label/name_look_and_feel.hpp"
#include "rotary_slider_look_and_feel.hpp"

namespace zlInterface {
    class RotarySlider : public juce::Component {
    public:
        explicit RotarySlider(const juce::String &labelText, UIBase &base) :
        myLookAndFeel(base), nameLookAndFeel(base), uiBase(base){
            // setup slider
            slider.setSliderStyle(juce::Slider::Rotary);
            slider.setTextBoxIsEditable(false);
            slider.setDoubleClickReturnValue(true, 0.0);
            slider.setLookAndFeel(&myLookAndFeel);
            slider.setScrollWheelEnabled(true);
            addAndMakeVisible(slider);

            // setup label
            label.setText(labelText, juce::dontSendNotification);
            label.setLookAndFeel(&nameLookAndFeel);
            addAndMakeVisible(label);
        }

        ~RotarySlider() override {
            slider.setLookAndFeel(nullptr);
            label.setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            auto boundMinWH = juce::jmin(bound.getWidth(), bound.getHeight() - uiBase.getFontSize() * FontHuge);
            bound = bound.withSizeKeepingCentre(boundMinWH, boundMinWH + uiBase.getFontSize() * FontHuge);
            auto textBound = bound.removeFromTop(uiBase.getFontSize() * FontHuge);
            label.setBounds(textBound.toNearestInt());
            auto bounds = bound;
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.9f;
            auto buttonBounds = bounds.withSizeKeepingCentre(radius, radius);
            slider.setBounds(buttonBounds.toNearestInt());
        }

        void paint(juce::Graphics &g) override {
            juce::ignoreUnused(g);
        }

        juce::Slider &getSlider() { return slider; }

        juce::Label &getLabel() { return label; }

        void setEditable(bool f) {
            myLookAndFeel.setEditable(f);
            nameLookAndFeel.setEditable(f);
        }

    private:
        RotarySliderLookAndFeel myLookAndFeel;
        NameLookAndFeel nameLookAndFeel;
        juce::Slider slider;
        juce::Label label;

        UIBase &uiBase;

        constexpr static float sliderHeight = 0.85f;
        constexpr static float labelHeight = 1.f - sliderHeight;
    };
}
#endif //ZL_ROTARY_SLIDER_COMPONENT_H
