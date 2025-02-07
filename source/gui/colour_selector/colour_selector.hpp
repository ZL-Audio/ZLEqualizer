// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COLOUR_SELECTOR_H
#define ZL_COLOUR_SELECTOR_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../interface_definitions.hpp"
#include "../calloutbox/call_out_box_laf.hpp"

namespace zlInterface {
    class ColourSelector final : public juce::Component,
                                 private juce::ChangeListener {
    public:
        explicit ColourSelector(zlInterface::UIBase &base, juce::Component &parent,
                                float widthS = 12.f, float heightS = 10.f);

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void setColour(const juce::Colour c) {
            colour = c;
            repaint();
        }

        void setOpacity(const float x) {
            colour = colour.withAlpha(x);
            repaint();
        }

        [[nodiscard]] juce::Colour getColour() const {
            return colour;
        }

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CallOutBoxLAF laf;
        juce::Component &parentC;
        float selectorWidthS, selectorHeightS;

        void changeListenerCallback(juce::ChangeBroadcaster *source) override;

        juce::Colour colour = juce::Colours::red;
    };
} // zlInterface

#endif //ZL_COLOUR_SELECTOR_H
