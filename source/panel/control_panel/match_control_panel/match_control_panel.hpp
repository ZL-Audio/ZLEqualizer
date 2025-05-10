// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "BinaryData.h"

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../panel_definitons.hpp"
#include "match_runner.hpp"

namespace zlpanel {
    class MatchControlPanel final : public juce::Component,
                                    private juce::ValueTree::Listener {
    public:
        explicit MatchControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~MatchControlPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlgui::UIBase &ui_base_;
        zldsp::eq_match::EqMatchAnalyzer<double> &analyzer_;

        const std::unique_ptr<juce::Drawable> start_drawable_, pause_drawable_, save_drawable_;

        zlgui::CompactCombobox side_choose_box_, fit_algo_box_;
        zlgui::CompactLinearSlider weight_slider_, smooth_slider_, slope_slider_;
        zlgui::CompactLinearSlider num_band_slider_;
        zlgui::ClickButton learn_button_, save_button_, fit_button_;

        juce::Rectangle<int> internal_bound_;

        MatchRunner match_runner_;

        std::unique_ptr<juce::FileChooser> chooser_;
        inline auto static const kPresetDirectory =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile(JucePlugin_Name)
                .getChildFile("Match Presets");

        void resetDefault();

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;

        void loadFromPreset();

        void saveToPreset();
    };
} // zlpanel
