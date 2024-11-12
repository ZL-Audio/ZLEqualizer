// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_NAME_LOOK_AND_FEEL_H
#define ZL_NAME_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"

namespace zlInterface {
    class NameLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit NameLookAndFeel(UIBase &base) {
            uiBase = &base;
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (label.isBeingEdited()) {
                return;
            }
            if (editable) {
                g.setColour(uiBase->getTextColor().withMultipliedAlpha(alpha.load()));
            } else {
                g.setColour(uiBase->getTextInactiveColor().withMultipliedAlpha(alpha.load()));
            }
            g.setFont(uiBase->getFontSize() * fontScale.load());
            auto bound = label.getLocalBounds().toFloat();
            bound.removeFromTop(uPadding.load());
            bound.removeFromBottom(dPadding.load());
            bound.removeFromLeft(lPadding.load());
            bound.removeFromRight(rPadding.load());
            g.drawText(label.getText(), bound, justification.load());
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline void setAlpha(const float x) { alpha.store(x); }

        inline void setFontScale(const float x) { fontScale.store(x); }

        inline void setJustification(const juce::Justification j) { justification.store(j); }

        inline void setPadding(const float l, const float r, const float u, const float d) {
            lPadding.store(l);
            rPadding.store(r);
            uPadding.store(u);
            dPadding.store(d);
        }

    private:
        std::atomic<bool> editable{true};
        std::atomic<float> alpha{1.f};
        std::atomic<float> fontScale{FontNormal};
        std::atomic<juce::Justification> justification{juce::Justification::centred};
        std::atomic<float> lPadding{0.f}, rPadding{0.f}, uPadding{0.f}, dPadding{0.f};

        UIBase *uiBase;
    };
}

#endif //ZL_NAME_LOOK_AND_FEEL_H
