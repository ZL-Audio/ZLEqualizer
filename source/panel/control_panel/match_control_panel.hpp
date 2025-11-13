// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../multilingual/tooltip_helper.hpp"

#include "control_background.hpp"
#include "match_runner.hpp"

namespace zlpanel {
    class MatchControlPanel final : public juce::Component {
    public:
        explicit MatchControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                   const multilingual::TooltipHelper& tooltip_helper);

        int getIdealHeight() const;

        int getIdealWidth() const;

        void resized() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        ControlBackground control_background_;

        const std::unique_ptr<juce::Drawable> save_drawable_;
        zlgui::button::ClickButton save_button_;

        zlgui::combobox::CompactCombobox target_box_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label shift_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> shift_slider_;

        juce::Label smooth_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> smooth_slider_;

        juce::Label slope_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> slope_slider_;

        const std::unique_ptr<juce::Drawable> start_drawable_;
        zlgui::button::ClickButton fit_start_button_;

        zlgui::slider::CompactLinearSlider<false, false, false> num_band_slider_;

        std::unique_ptr<juce::FileChooser> chooser_;
        const juce::File kPresetDirectory =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile(JucePlugin_Name)
                .getChildFile("Match Presets");
        std::vector<float> preset_freqs_, preset_dbs_{};

        MatchRunner match_runner_;

        void visibilityChanged() override;

        void saveToPreset();

        void loadFromPreset();
    };
}
