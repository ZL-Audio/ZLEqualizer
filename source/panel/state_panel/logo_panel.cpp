// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "logo_panel.hpp"

namespace zlpanel {
    LogoPanel::LogoPanel(PluginProcessor &p,
                         zlgui::UIBase &base,
                         UISettingPanel &uiSettingPanel)
        : state_ref_(p.state_),
          ui_base_(base), panel_to_show_(uiSettingPanel),
          brand_drawable_(juce::Drawable::createFromImageData(BinaryData::zlaudio_svg, BinaryData::zlaudio_svgSize)),
          logo_drawable_(juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize)) {
        juce::ignoreUnused(state_ref_);
        SettableTooltipClient::setTooltip(ui_base_.getToolTipText(zlgui::multilingual::Labels::kPluginLogo));
        setBufferedToImage(true);
    }

    void LogoPanel::paint(juce::Graphics &g) {
        const auto temp_brand = brand_drawable_->createCopy();
        const auto temp_logo = logo_drawable_->createCopy();
        temp_brand->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor());
        temp_logo->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor());
        temp_logo->replaceColour(
            juce::Colour(static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), .5f),
            ui_base_.getTextColor().withMultipliedAlpha(.5f));

        auto bound = getLocalBounds().toFloat();
        const auto padding = juce::jmin(ui_base_.getFontSize() * 0.5f, ui_base_.getFontSize() * 0.5f);
        bound = bound.withSizeKeepingCentre(bound.getWidth() - padding, bound.getHeight() - padding);

        auto bound_to_use = juce::Rectangle<float>(bound.getWidth(), ui_base_.getFontSize() * 2.f);
        bound = justification_.appliedToRectangle(bound_to_use, bound);

        const auto logo_woh = static_cast<float>(logo_drawable_->getWidth()) /
                             static_cast<float>(logo_drawable_->getHeight());
        const auto brand_woh = static_cast<float>(brand_drawable_->getWidth()) /
                              static_cast<float>(brand_drawable_->getHeight());
        const auto woh = logo_woh + brand_woh + 0.1f;
        const auto width = juce::jmin(bound.getWidth(), bound.getHeight() * woh);
        const auto height = juce::jmin(bound.getHeight(), bound.getWidth() / woh);

        bound_to_use = juce::Rectangle<float>(width, height);
        bound = justification_.appliedToRectangle(bound_to_use, bound);

        temp_brand->setTransform(
            juce::AffineTransform::scale(bound.getHeight() / static_cast<float>(brand_drawable_->getHeight())));
        temp_brand->drawAt(g, bound.getX(), bound.getY(), 1.0f);

        temp_logo->setTransform(
            juce::AffineTransform::scale(bound.getHeight() / static_cast<float>(logo_drawable_->getHeight())));
        temp_logo->drawAt(g, bound.getX() + bound.getHeight() * (woh - logo_woh), bound.getY(), 1.0f);
    }

    void LogoPanel::setJustification(const int justificationFlags) {
        justification_ = justificationFlags;
    }

    void LogoPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        ui_base_.closeAllBox();
        if (event.mods.isCommandDown()) {
            getParentComponent()->getParentComponent()->getParentComponent()->setSize(
                static_cast<int>(zlstate::windowW::defaultV),
                static_cast<int>(zlstate::windowH::defaultV));
        } else {
            panel_to_show_.loadSetting();
            panel_to_show_.setVisible(true);
        }
    }
} // zlpanel
