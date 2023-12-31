//
// Created by Zishu Liu on 12/29/23.
//

#ifndef COMPACT_BUTTON_H
#define COMPACT_BUTTON_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <friz/friz.h>

#include "../../interface_definitions.h"
#include "compact_button_look_and_feel.h"

namespace zlInterface {
    class CompactButton : public juce::Component {
    public:
        explicit CompactButton(const juce::String &labelText, UIBase &base);

        ~CompactButton() override;

        void resized() override;

        void buttonDownAnimation();

        inline void setEditable(const bool f) {
            lookAndFeel.setEditable(f);
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
