// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLPANEL_LOUDNESS_DISPLAY_HPP
#define ZLPANEL_LOUDNESS_DISPLAY_HPP

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {
    class LoudnessDisplay final : public juce::Component {
    public:
        explicit LoudnessDisplay(PluginProcessor &p, zlInterface::UIBase &base);

        void paint(juce::Graphics &g) override;

        void attachGroup(const size_t idx) {
            bandIdx = idx;
        }

        void checkRepaint();

    private:
        PluginProcessor &processorRef;
        zlInterface::UIBase &uiBase;
        juce::Time previousTime{};

        size_t bandIdx{0};
        std::array<juce::RangedAudioParameter *, zlState::bandNUM> isDynamicOnParas{};
        std::array<juce::RangedAudioParameter *, zlState::bandNUM> isThresholdAutoParas{};
        bool shouldPaint{false};
    };
} // zlPanel

#endif //ZLPANEL_LOUDNESS_DISPLAY_HPP
