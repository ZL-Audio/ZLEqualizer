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
    class OtherUISettingPanel final : public juce::Component {
    public:
        static constexpr float heightP = 4.f * 7.f;

        explicit OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void resized() override;

        void setRendererList(const juce::StringArray &rendererList);

    private:
        PluginProcessor &pRef;
        zlgui::UIBase &ui_base_;
        zlgui::NameLookAndFeel nameLAF;

        juce::Label renderingEngineLabel;
        zlgui::CompactCombobox renderingEngineBox;
        juce::Label refreshRateLabel;
        zlgui::CompactCombobox refreshRateBox;
        juce::Label fftLabel;
        zlgui::CompactLinearSlider fftTiltSlider, fftSpeedSlider;
        zlgui::CompactCombobox fftOrderBox;
        juce::Label curveThickLabel;
        zlgui::CompactLinearSlider singleCurveSlider, sumCurveSlider;
        juce::Label defaultPassFilterSlopeLabel;
        zlgui::CompactCombobox defaultPassFilterSlopeBox;
        juce::Label dynLinkLabel;
        zlgui::CompactCombobox dynLinkBox;
        juce::Label tooltipLabel;
        zlgui::CompactCombobox tooltipONBox, tooltipLangBox;
    };
} // zlpanel
