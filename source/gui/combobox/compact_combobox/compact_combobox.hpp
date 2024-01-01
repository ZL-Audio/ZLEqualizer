//
// Created by Zishu Liu on 12/30/23.
//

#ifndef COMPACT_COMBOBOX_H
#define COMPACT_COMBOBOX_H

#include "compact_combobox_look_and_feel.hpp"

namespace zlInterface {
    class CompactCombobox : public juce::Component {
    public:
        CompactCombobox(const juce::String &labelText, const juce::StringArray &choices, UIBase &base);

        ~CompactCombobox() override;

        void resized() override;

        inline void setEditable(const bool f) {
            boxLookAndFeel.setEditable(f);
        }

    private:
        CompactComboboxLookAndFeel boxLookAndFeel;
        juce::ComboBox comboBox;
    };
}


#endif //COMPACT_COMBOBOX_H
