//
// Created by Zishu Liu on 12/28/23.
//

#ifndef TWO_VALUE_ROTARY_SLIDER_COMPONENT_H
#define TWO_VALUE_ROTARY_SLIDER_COMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <friz/friz.h>

#include "../../interface_definitions.hpp"
#include "first_rotary_slider_look_and_feel.hpp"
#include "second_rotary_slider_look_and_feel.hpp"
#include "../../label/name_look_and_feel.hpp"

namespace zlInterface {
    class TwoValueRotarySlider : public juce::Component,
    private juce::Label::Listener, private juce::Slider::Listener {
    public:
        explicit TwoValueRotarySlider(const juce::String &labelText, UIBase &base);

        ~TwoValueRotarySlider() override;

        // void paintOverChildren(juce::Graphics &g) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void resized() override;

        inline juce::Slider &getSlider1() { return slider1; }

        inline juce::Slider &getSlider2() { return slider2; }

        void setShowSlider2(bool x);

        inline void setPadding(const float lr, const float ub) {
            lrPad.store(lr);
            ubPad.store(ub);
        }

        inline void setEditable(const bool x) {
            editable.store(x);
            labelLookAndFeel.setEditable(x);
            labelLookAndFeel1.setEditable(x);
            labelLookAndFeel2.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline bool getEditable() const { return editable.load(); }

    private:
        UIBase &uiBase;

        juce::Slider slider1, slider2;
        FirstRotarySliderLookAndFeel slider1LAF;
        SecondRotarySliderLookAndFeel slider2LAF;

        juce::Label label, label1, label2;
        NameLookAndFeel labelLookAndFeel, labelLookAndFeel1, labelLookAndFeel2;

        NameLookAndFeel textBoxLAF;

        std::atomic<bool> showSlider2 = true, mouseOver, editable;
        std::atomic<float> lrPad = 0, ubPad = 0;

        friz::Animator animator;
        static constexpr int animationId = 1;

        static juce::String getDisplayValue(juce::Slider &s);

        void labelTextChanged(juce::Label *labelThatHasChanged) override;

        void editorShown(juce::Label *l, juce::TextEditor &editor) override;

        void editorHidden(juce::Label *l, juce::TextEditor &editor) override;

        void sliderValueChanged(juce::Slider *slider) override;
    };
}

#endif //TWO_VALUE_ROTARY_SLIDER_COMPONENT_H
