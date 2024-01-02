//
// Created by Zishu Liu on 12/29/23.
//

#ifndef COMPACT_BUTTON_LOOK_AND_FEEL_H
#define COMPACT_BUTTON_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class CompactButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
            g.fillAll(uiBase.getBackgroundColor());

            auto bounds = button.getLocalBounds().toFloat();
            bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.5f, {});
            bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f, {.flip = true});
            if (button.getToggleState()) {
                bounds = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.25f, {});
                uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.5f, {
                                                  .darkShadowColor = uiBase.getDarkShadowColor().
                                                  withMultipliedAlpha(buttonDepth),
                                                  .brightShadowColor = uiBase.getBrightShadowColor().
                                                  withMultipliedAlpha(buttonDepth),
                                                  .changeDark = true,
                                                  .changeBright = true
                                              });
            }
            if (editable.load()) {
                auto textBound = button.getLocalBounds().toFloat();
                if (button.getToggleState()) {
                    g.setColour(uiBase.getTextColor().withAlpha(0.5f + buttonDepth * 0.5f));
                } else {
                    g.setColour(uiBase.getTextColor().withAlpha(0.5f));
                }
                g.setFont(uiBase.getFontSize() * FontLarge);
                g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
            }
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline float getDepth() { return buttonDepth.load(); }

        inline void setDepth(const float x) { buttonDepth = x; }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> buttonDepth = 0.f;

        UIBase &uiBase;
    };
}

#endif //COMPACT_BUTTON_LOOK_AND_FEEL_H
