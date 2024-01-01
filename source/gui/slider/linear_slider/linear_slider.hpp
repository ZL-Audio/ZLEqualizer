// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_LINEAR_SLIDER_H
#define ZL_LINEAR_SLIDER_H

#include "../../label/name_look_and_feel.hpp"
#include "linear_slider_look_and_feel.hpp"

namespace zlInterface {
    class LinearSlider : public juce::Component {
    public:
        explicit LinearSlider(const juce::String &labelText, UIBase &base) :
                myLookAndFeel(base), nameLookAndFeel(base) {
            uiBase = &base;
            // setup slider
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
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

        ~LinearSlider() override {
            slider.setLookAndFeel(nullptr);
            label.setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            auto labelBound = bound.removeFromTop(labelHeight * bound.getHeight());
            label.setBounds(labelBound.toNearestInt());
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getWidth() * sliderRatio);
            slider.setBounds(bound.toNearestInt());
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
        LinearSliderLookAndFeel myLookAndFeel;
        NameLookAndFeel nameLookAndFeel;
        juce::Slider slider;
        juce::Label label;

        constexpr static float sliderHeight = 0.7f;
        constexpr static float labelHeight = 1.f - sliderHeight;
        constexpr static float sliderRatio = 0.45f;

        UIBase *uiBase;
    };
}

#endif //ZL_LINEAR_SLIDER_H
