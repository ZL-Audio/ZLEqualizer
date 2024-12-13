// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef MATCH_CONTROL_PANEL_HPP
#define MATCH_CONTROL_PANEL_HPP

#include "BinaryData.h"

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {
    class MatchControlPanel final : public juce::Component,
                                    private juce::ValueTree::Listener {
    public:
        explicit MatchControlPanel(PluginProcessor &p, zlInterface::UIBase &base);

        ~MatchControlPanel() override;

        void paint (juce::Graphics &g) override;

        void resized() override;

    private:
        static constexpr float weightP = 0.365f;
        zlInterface::UIBase &uiBase;
        const std::unique_ptr<juce::Drawable> startDrawable, pauseDrawable, saveDrawable;

        zlInterface::CompactCombobox sideChooseBox, fitAlgoBox;
        zlInterface::CompactLinearSlider weightSlider, smoothSlider, slopeSlider, numBandSlider;
        zlInterface::ClickButton learnButton, saveButton, fitButton;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;
    };
} // zlPanel

#endif //MATCH_CONTROL_PANEL_HPP
