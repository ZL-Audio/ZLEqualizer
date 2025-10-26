// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "logo_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    LogoPanel::LogoPanel(PluginProcessor&, zlgui::UIBase& base, multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        brand_drawable_(juce::Drawable::createFromImageData(BinaryData::zlaudio_svg, BinaryData::zlaudio_svgSize)),
        logo_drawable_(juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize)) {
        juce::ignoreUnused(tooltip_helper);
        setAlpha(.5f);
    }

    void LogoPanel::paint(juce::Graphics& g) {
        const auto temp_brand = brand_drawable_->createCopy();
        const auto temp_logo = logo_drawable_->createCopy();
        temp_brand->replaceColour(juce::Colours::black, base_.getTextColour());
        temp_logo->replaceColour(juce::Colours::black, base_.getTextColour());
        temp_logo->replaceColour(juce::Colours::black.withAlpha(.5f), base_.getTextColour().withMultipliedAlpha(.5f));

        const auto padding = getPaddingSize(base_.getFontSize());

        auto bound = getLocalBounds();

        const auto bound1 = bound.removeFromLeft(bound.getHeight()).toFloat();
        temp_brand->drawWithin(g, bound1, juce::RectanglePlacement::centred, 1.f);

        bound.removeFromLeft(padding);
        const auto bound2 = bound.removeFromLeft(bound.getHeight()).toFloat();
        temp_logo->drawWithin(g, bound2, juce::RectanglePlacement::centred, 1.f);
    }

    void LogoPanel::mouseEnter(const juce::MouseEvent&) {
        setAlpha(1.f);
    }

    void LogoPanel::mouseExit(const juce::MouseEvent&) {
        setAlpha(.5f);
    }

    void LogoPanel::mouseDoubleClick(const juce::MouseEvent&) {
        base_.setPanelProperty(zlgui::kUISettingPanel, true);
    }
}
