// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class ColourSettingPanel final : public juce::Component {
    public:
        static constexpr float kHeightP = 56.f;

        explicit ColourSettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~ColourSettingPanel() override;

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        PluginProcessor &pRef;
        zlgui::UIBase &ui_base_;
        zlgui::NameLookAndFeel name_laf_;
        zlgui::ColourOpacitySelector text_selector_, background_selector_, shadow_selector_, glow_selector_;
        zlgui::ColourOpacitySelector pre_selector_, post_selector_, side_selector_, grid_selector_, tag_selector_;
        zlgui::ColourOpacitySelector gain_selector_, side_loudness_selector_;
        static constexpr size_t kNumSelectors = 11;
        std::array<juce::Label, kNumSelectors> selector_labels_;
        std::array<zlgui::ColourOpacitySelector *, kNumSelectors> selectors_{
            &text_selector_,
            &background_selector_,
            &shadow_selector_,
            &glow_selector_,
            &pre_selector_,
            &post_selector_,
            &side_selector_,
            &grid_selector_,
            &tag_selector_,
            &gain_selector_,
            &side_loudness_selector_
        };
        static constexpr std::array kSelectorNames{
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
            "Side Loudness Colour"
        };

        static constexpr std::array<zlgui::ColourIdx, kNumSelectors> kColourIdx{
            zlgui::ColourIdx::kTextColour,
            zlgui::ColourIdx::kBackgroundColour,
            zlgui::ColourIdx::kShadowColour,
            zlgui::ColourIdx::kGlowColour,
            zlgui::ColourIdx::kPreColour,
            zlgui::ColourIdx::kPostColour,
            zlgui::ColourIdx::kSideColour,
            zlgui::ColourIdx::kGridColour,
            zlgui::ColourIdx::kTagColour,
            zlgui::ColourIdx::kGainColour,
            zlgui::ColourIdx::kSideLoudnessColour
        };

        static constexpr std::array kTagNames{
            "text_colour",
            "background_colour",
            "shadow_colour",
            "glow_colour",
            "pre_colour",
            "post_colour",
            "side_colour",
            "grid_colour",
        };

        juce::Label c_map1_label_, c_map2_label_;
        zlgui::ColourMapSelector c_map1_selector_, c_map2_selector_;
        juce::Label import_label_, export_label_;

        std::unique_ptr<juce::FileChooser> chooser_;
        inline auto static const kSettingDirectory =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile("Shared Settings");
    };
} // zlpanel
