// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_opacity_selector.hpp"

namespace zlgui {
    ColourOpacitySelector::ColourOpacitySelector(zlgui::UIBase &base, juce::Component &parent,
                                                 const bool use_opacity,
                                                 const float width_s, const float height_s,
                                                 const float w1, const float w2)
        : ui_base_(base),
          selector_(base, parent, width_s, height_s),
          slider_("Opacity", base),
          opacity_on_(use_opacity) {
        weights_ = {w1, w2};
        if (opacity_on_) {
            slider_.getSlider().setRange(0.0, 1.0, 0.01);
            slider_.getSlider().addListener(this);
            addAndMakeVisible(slider_);
        }
        addAndMakeVisible(selector_);
    }

    ColourOpacitySelector::~ColourOpacitySelector() {
        if (opacity_on_) {
            slider_.getSlider().removeListener(this);
        }
    }

    void ColourOpacitySelector::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            ui_base_.getFontSize() * kFontLarge * 1.75f);
        const auto padding = ui_base_.getFontSize() * 2.f;
        const auto width = bound.getWidth() - padding;
        selector_.setBounds(bound.removeFromLeft(width * weights_[0]).toNearestInt());
        if (opacity_on_) {
            bound.removeFromLeft(padding);
            slider_.setBounds(bound.removeFromLeft(width * weights_[1]).toNearestInt());
        }
    }

    void ColourOpacitySelector::sliderValueChanged(juce::Slider *s) {
        selector_.setOpacity(static_cast<float>(s->getValue()));
    }
} // zlgui
