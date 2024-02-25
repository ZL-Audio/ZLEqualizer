// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "logo_panel.hpp"

namespace zlPanel {
    LogoPanel::LogoPanel(juce::AudioProcessorValueTreeState &state, zlInterface::UIBase &base)
        : stateRef(state),
          uiBase(base),
          brandDrawable(juce::Drawable::createFromImageData(BinaryData::zlaudio_svg, BinaryData::zlaudio_svgSize)),
          logoDrawable(juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize)) {
        // setBufferedToImage(true);
    }

    void LogoPanel::paint(juce::Graphics &g) {
        const auto tempBrand = brandDrawable->createCopy();
        const auto tempLogo = logoDrawable->createCopy();
        tempBrand->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
        tempLogo->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
        tempLogo->replaceColour(
            juce::Colour(static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), .5f),
            uiBase.getTextColor().withMultipliedAlpha(.5f));

        auto bound = getLocalBounds().toFloat();
        const auto padding = juce::jmin(uiBase.getFontSize() * 0.5f, uiBase.getFontSize() * 0.5f);
        bound = bound.withSizeKeepingCentre(bound.getWidth() - padding, bound.getHeight() - padding);

        auto boundToUse = juce::Rectangle<float>(bound.getWidth(), uiBase.getFontSize() * 2.f);
        bound = justification.appliedToRectangle(boundToUse, bound);

        const auto logoWOH = static_cast<float>(logoDrawable->getWidth()) /
                             static_cast<float>(logoDrawable->getHeight());
        const auto brandWOH = static_cast<float>(brandDrawable->getWidth()) /
                              static_cast<float>(brandDrawable->getHeight());
        const auto widthOverHeight = logoWOH + brandWOH + 0.1f;
        const auto width = juce::jmin(bound.getWidth(), bound.getHeight() * widthOverHeight);
        const auto height = juce::jmin(bound.getHeight(), bound.getWidth() / widthOverHeight);

        boundToUse = juce::Rectangle<float>(width, height);
        bound = justification.appliedToRectangle(boundToUse, bound);

        tempBrand->setTransform(
            juce::AffineTransform::scale(bound.getHeight() / static_cast<float>(brandDrawable->getHeight())));
        tempBrand->drawAt(g, bound.getX(), bound.getY(), 1.0f);

        tempLogo->setTransform(
            juce::AffineTransform::scale(bound.getHeight() / static_cast<float>(logoDrawable->getHeight())));
        tempLogo->drawAt(g, bound.getX() + bound.getHeight() * (widthOverHeight - logoWOH), bound.getY(), 1.0f);
    }

    void LogoPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        auto styleID = static_cast<size_t>(stateRef.getRawParameterValue(zlState::uiStyle::ID)->load());
        styleID = (styleID + 1) % (zlState::uiStyle::maxV + 1);
        uiBase.setStyle(styleID);
        auto *para = stateRef.getParameter(zlState::uiStyle::ID);
        para->beginChangeGesture();
        para->setValueNotifyingHost(zlState::uiStyle::convertTo01(static_cast<float>(styleID)));
        para->endChangeGesture();
    }

    void LogoPanel::setJustification(const int justificationFlags) {
        justification = justificationFlags;
    }
} // zlPanel
