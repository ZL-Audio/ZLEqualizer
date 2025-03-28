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

namespace zlPanel {
    class OtherUISettingPanel final : public juce::Component {
    public:
        static constexpr float heightP = 4.f * 7.f;

        explicit OtherUISettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

        void setRendererList(const juce::StringArray &rendererList);

    private:
        PluginProcessor &pRef;
        zlInterface::UIBase &uiBase;
        zlInterface::NameLookAndFeel nameLAF;

        juce::Label renderingEngineLabel;
        zlInterface::CompactCombobox renderingEngineBox;
        juce::Label refreshRateLabel;
        zlInterface::CompactCombobox refreshRateBox;
        juce::Label fftLabel;
        zlInterface::CompactLinearSlider fftTiltSlider, fftSpeedSlider;
        zlInterface::CompactCombobox fftOrderBox;
        juce::Label curveThickLabel;
        zlInterface::CompactLinearSlider singleCurveSlider, sumCurveSlider;
        juce::Label defaultPassFilterSlopeLabel;
        zlInterface::CompactCombobox defaultPassFilterSlopeBox;
        juce::Label dynLinkLabel;
        zlInterface::CompactCombobox dynLinkBox;
        juce::Label tooltipLabel;
        zlInterface::CompactCombobox tooltipONBox, tooltipLangBox;
    };
} // zlPanel
