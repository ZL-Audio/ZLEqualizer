// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COLOUR_MAP_SELECTOR_HPP
#define ZL_COLOUR_MAP_SELECTOR_HPP

#include <juce_gui_basics/juce_gui_basics.h>
#include "../interface_definitions.hpp"
#include "../combobox/combobox.hpp"

namespace zlInterface {
    class ColourMapSelector final : public juce::Component,
                                    private juce::ComboBox::Listener {
    public:
        explicit ColourMapSelector(zlInterface::UIBase &base, float boxWidth = .5f);

        void paint(juce::Graphics &g) override;

        void resized() override;

        juce::ComboBox &getBox() { return mapBox.getBox(); }

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CompactCombobox mapBox;
        float mapBoxWidthP{.5f};

        void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;
    };
} // zlInterface

#endif //ZL_COLOUR_MAP_SELECTOR_HPP
