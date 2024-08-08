// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef INTERNAL_SETTING_PANEL_HPP
#define INTERNAL_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {
    class InternalSettingPanel final : public juce::Component {
    public:
        explicit InternalSettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

        ~InternalSettingPanel() override;

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
        juce::Label wheelLabel;
        zlInterface::CompactLinearSlider roughWheelSlider, fineWheelSlider;
        juce::Label rotaryStyleLabel;
        zlInterface::CompactCombobox rotaryStyleBox;
        zlInterface::CompactLinearSlider rotaryDragSensitivitySlider;
        juce::Label refreshRateLabel;
        zlInterface::CompactCombobox refreshRateBox;
        juce::Label fftLabel;
        zlInterface::CompactLinearSlider fftTiltSlider, fftSpeedSlider;
        juce::Label curveThickLabel;
        zlInterface::CompactLinearSlider singleCurveSlider, sumCurveSlider;

        static constexpr size_t numSelectors = 10;
        std::array<juce::Label, numSelectors> selectorLabels;
        std::array<zlInterface::ColourOpacitySelector*, numSelectors> selectors{
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
    };
} // zlPanel

#endif //INTERNAL_SETTING_PANEL_HPP
