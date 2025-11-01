// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../helper/panel_constants.hpp"

namespace zlpanel {
    class ColourSettingPanel final : public juce::Component {
    public:
        static constexpr float kHeightP = 56.f;

        explicit ColourSettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~ColourSettingPanel() override;

        void loadSetting();

        void saveSetting();

        void resetSetting();

        int getIdealHeight() const;

        void resized() override;

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        PluginProcessor &pRef;
        zlgui::UIBase &base_;
        zlgui::label::NameLookAndFeel name_laf_;

        static constexpr size_t kNumSelectors = static_cast<size_t>(zlgui::ColourIdx::kColourNum);
        std::array<juce::Label, kNumSelectors> selector_labels_;
        std::array<std::unique_ptr<zlgui::colour_selector::ColourOpacitySelector>, kNumSelectors> selectors_;

        juce::Label c_map1_label_, c_map2_label_;
        zlgui::colour_selector::ColourMapSelector c_map1_selector_, c_map2_selector_;

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
