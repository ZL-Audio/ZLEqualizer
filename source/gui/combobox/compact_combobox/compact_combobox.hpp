//
// Created by Zishu Liu on 12/30/23.
//

#ifndef COMPACT_COMBOBOX_H
#define COMPACT_COMBOBOX_H

#include <friz/friz.h>

#include "compact_combobox_look_and_feel.hpp"

namespace zlInterface {
    class CompactCombobox : public juce::Component {
    public:
        CompactCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base);

        ~CompactCombobox() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        inline void setEditable(const bool f) {
            boxLookAndFeel.setEditable(f);
        }

    private:
        CompactComboboxLookAndFeel boxLookAndFeel;
        juce::ComboBox comboBox;

        friz::Animator animator;
        static constexpr int animationId = 1;
    };
}


#endif //COMPACT_COMBOBOX_H
