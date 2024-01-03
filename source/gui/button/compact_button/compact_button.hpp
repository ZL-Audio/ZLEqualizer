//
// Created by Zishu Liu on 12/29/23.
//

#ifndef COMPACT_BUTTON_H
#define COMPACT_BUTTON_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <friz/friz.h>

#include "../../interface_definitions.hpp"
#include "compact_button_look_and_feel.hpp"

namespace zlInterface {
    class CompactButton : public juce::Component {
    public:
        explicit CompactButton(const juce::String &labelText, UIBase &base);

        ~CompactButton() override;

        void resized() override;

        void buttonDownAnimation();

        inline juce::ToggleButton& getButton() {return button;}

        inline void setEditable(const bool x) {
            lookAndFeel.setEditable(x);
            button.setInterceptsMouseClicks(x, false);
        }

    private:
        UIBase &uiBase;

        juce::ToggleButton button;
        CompactButtonLookAndFeel lookAndFeel;
        static constexpr int animationId = 1;

        friz::Animator animator;
    };
}


#endif //COMPACT_BUTTON_H
