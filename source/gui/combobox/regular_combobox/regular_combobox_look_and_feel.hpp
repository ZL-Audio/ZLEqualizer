// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZL_REGULAR_COMBOBOX_LOOK_AND_FEEL_H
#define ZL_REGULAR_COMBOBOX_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class RegularComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        // rounded menu box
        RegularComboboxLookAndFeel(UIBase &base) {
            uiBase = &base;
            setColour(juce::PopupMenu::backgroundColourId, uiBase->getBackgroundInactiveColor());
        }

        void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox &box) override {
            juce::ignoreUnused(isButtonDown);
            auto cornerSize = uiBase->getFontSize() * 0.5f;
            if (!box.isPopupActive()) {
                auto boxBounds = juce::Rectangle<float>(0, 0,
                                                        static_cast<float>(width),
                                                        static_cast<float>(height) * 1.0f);
                boxBounds = uiBase->fillRoundedShadowRectangle(g, boxBounds, cornerSize, {});
                uiBase->fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blurRadius = 0.45f, .flip = true});
            } else {
                auto boxBounds = juce::Rectangle<float>(0, 0,
                                                        static_cast<float>(width),
                                                        static_cast<float>(height) * 2 + cornerSize * 3.f);
                boxBounds = uiBase->fillRoundedShadowRectangle(g, boxBounds, cornerSize,
                                                               {.curveBottomLeft = false, .curveBottomRight = false});
                uiBase->fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blurRadius = 0.45f, .flip = true});
            }
        }

        void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
            label.setBounds(0, 0, box.getWidth(), box.getHeight());
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (editable.load()) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            g.setFont(uiBase->getFontSize() * FontLarge);
            g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred);
        }

        void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override {
            auto cornerSize = uiBase->getFontSize() * 0.5f;
            auto boxBounds = juce::Rectangle<float>(0, -3.f * cornerSize, static_cast<float>(width),
                                                    static_cast<float>(height) + 3.f * cornerSize);
            boxBounds = uiBase->fillRoundedShadowRectangle(g, boxBounds, cornerSize,
                                                           {.curveTopLeft = false, .curveTopRight = false});
            uiBase->fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blurRadius = 0.45f, .flip = true});
            g.setColour(uiBase->getTextInactiveColor());
            g.fillRect(boxBounds.getX(), 0.0f, boxBounds.getWidth(), cornerSize * 0.15f);
        }

        void getIdealPopupMenuItemSize(const juce::String &text, const bool isSeparator, int standardMenuItemHeight,
                                       int &idealWidth, int &idealHeight) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            idealWidth = static_cast<int>(0);
            idealHeight = static_cast<int>(uiBase->getFontSize() * FontLarge * 1.2f);
        }

        void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String &text,
                               const juce::String &shortcutKeyText, const juce::Drawable *icon,
                               const juce::Colour *const textColourToUse) override {
            juce::ignoreUnused(isSeparator, hasSubMenu, shortcutKeyText, icon, textColourToUse);
            if ((isHighlighted || isTicked) && isActive && editable) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            if (uiBase->getFontSize() > 0) {
                g.setFont(uiBase->getFontSize() * FontLarge);
            } else {
                g.setFont(static_cast<float>(area.getHeight()) * 0.35f);
            }
            auto center = area.toFloat().getCentre();
            g.drawSingleLineText(
                text,
                juce::roundToInt(center.x + g.getCurrentFont().getHorizontalScale()),
                juce::roundToInt(center.y),
                juce::Justification::horizontallyCentred);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return juce::roundToInt(uiBase->getFontSize() * 0.5f);
        }

        void setEditable(bool f) {
            editable.store(f);
        }

    private:
        std::atomic<bool> editable = true;

        UIBase *uiBase;
    };
}

#endif //ZL_REGULAR_COMBOBOX_LOOK_AND_FEEL_H
