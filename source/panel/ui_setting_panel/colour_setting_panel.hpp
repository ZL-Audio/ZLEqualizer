// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef COLOUR_UI_SETTING_PANEL_HPP
#define COLOUR_UI_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {
    class ColourSettingPanel final : public juce::Component {
    public:
        explicit ColourSettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

        ~ColourSettingPanel() override;

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

    private:
        PluginProcessor &pRef;
        zlInterface::UIBase &uiBase;
        zlInterface::NameLookAndFeel nameLAF;
        zlInterface::ColourOpacitySelector textSelector, backgroundSelector, shadowSelector, glowSelector;
        zlInterface::ColourOpacitySelector preSelector, postSelector, sideSelector, gridSelector, tagSelector;
        zlInterface::ColourOpacitySelector gainSelector;
        static constexpr size_t numSelectors = 10;
        std::array<juce::Label, numSelectors> selectorLabels;
        std::array<zlInterface::ColourOpacitySelector *, numSelectors> selectors{
            &textSelector,
            &backgroundSelector,
            &shadowSelector,
            &glowSelector,
            &preSelector,
            &postSelector,
            &sideSelector,
            &gridSelector,
            &tagSelector,
            &gainSelector
        };
        std::array<std::string, numSelectors> selectorNames{
            "Text Colour",
            "Background Colour",
            "Shadow Colour",
            "Glow Colour",
            "Pre Colour",
            "Post Colour",
            "Side Colour",
            "Grid Colour",
            "Tag Colour",
            "Gain Colour",
        };
        juce::Label cMap1Label, cMap2Label;
        zlInterface::ColourMapSelector cMap1Selector, cMap2Selector;
    };
} // zlPanel

#endif //COLOUR_UI_SETTING_PANEL_HPP
