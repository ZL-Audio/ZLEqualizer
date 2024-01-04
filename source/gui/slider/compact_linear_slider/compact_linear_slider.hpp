//
// Created by Zishu Liu on 12/30/23.
//

#ifndef COMPACT_LINEAR_SLIDER_H
#define COMPACT_LINEAR_SLIDER_H

#include <friz/friz.h>

#include "../../label/name_look_and_feel.hpp"
#include "compact_linear_slider_look_and_feel.hpp"

namespace zlInterface {
    class CompactLinearSlider : public juce::Component {
    public:
        explicit CompactLinearSlider(const juce::String &labelText, UIBase &base);

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

        inline juce::Slider& getSlider() {return slider;}

        inline void setEditable(const bool x) {
            nameLookAndFeel.setEditable(x);
            textLookAndFeel.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline void setPadding(const float lr, const float ub) {
            lrPad.store(lr);
            ubPad.store(ub);
        }

    private:
        UIBase &uiBase;

        CompactLinearSliderLookAndFeel sliderLookAndFeel;
        NameLookAndFeel nameLookAndFeel, textLookAndFeel;
        juce::Slider slider;
        juce::Label label, text;

        std::atomic<float> lrPad = 0, ubPad = 0;

        friz::Animator animator;
        static constexpr int animationId = 1;

        juce::String getDisplayValue(juce::Slider &s);
    };
}

#endif //COMPACT_LINEAR_SLIDER_H
