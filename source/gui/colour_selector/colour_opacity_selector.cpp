// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_opacity_selector.hpp"

namespace zlInterface {
    ColourOpacitySelector::ColourOpacitySelector(zlInterface::UIBase &base, juce::Component &parent,
                                                 const bool useOpacity,
                                                 const float widthS, const float heightS,
                                                 const float w1, const float w2)
        : uiBase(base),
          selector(base, parent, widthS, heightS),
          slider("Opacity", base),
          opacityON(useOpacity) {
        weights = {w1, w2};
        if (opacityON) {
            slider.getSlider().setRange(0.0, 1.0, 0.01);
            slider.getSlider().addListener(this);
            addAndMakeVisible(slider);
        }
        addAndMakeVisible(selector);
    }

    ColourOpacitySelector::~ColourOpacitySelector() {
        if (opacityON) {
            slider.getSlider().removeListener(this);
        }
    }

    void ColourOpacitySelector::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            uiBase.getFontSize() * FontLarge * 1.75f);
        const auto padding = uiBase.getFontSize() * 2.f;
        const auto width = bound.getWidth() - padding;
        selector.setBounds(bound.removeFromLeft(width * weights[0]).toNearestInt());
        if (opacityON) {
            bound.removeFromLeft(padding);
            slider.setBounds(bound.removeFromLeft(width * weights[1]).toNearestInt());
        }
    }

    void ColourOpacitySelector::sliderValueChanged(juce::Slider *s) {
        selector.setOpacity(static_cast<float>(s->getValue()));
    }
} // zlInterface
