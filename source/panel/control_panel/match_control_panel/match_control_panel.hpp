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

namespace zlPanel {
    class MatchControlPanel final : public juce::Component,
                                    private juce::ValueTree::Listener {
    public:
        explicit MatchControlPanel(PluginProcessor &p, zlInterface::UIBase &base);

        ~MatchControlPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        static constexpr float weightP = 0.05216f * 7.f;
        zlInterface::UIBase &uiBase;
        zlEqMatch::EqMatchAnalyzer<double> &analyzer;

        const std::unique_ptr<juce::Drawable> startDrawable, pauseDrawable, saveDrawable;

        zlInterface::CompactCombobox sideChooseBox, fitAlgoBox;
        zlInterface::CompactLinearSlider weightSlider, smoothSlider, slopeSlider;
        zlInterface::CompactLinearSlider numBandSlider;
        zlInterface::ClickButton learnButton, saveButton, fitButton;

        MatchRunner matchRunner;

        std::unique_ptr<juce::FileChooser> myChooser;
        inline auto static const presetDirectory =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile(JucePlugin_Name)
                .getChildFile("Match Presets");

        void resetDefault();

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;

        void loadFromPreset();

        void saveToPreset();
    };
} // zlPanel
