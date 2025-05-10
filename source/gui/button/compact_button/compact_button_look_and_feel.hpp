// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlgui {
    class CompactButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit CompactButtonLookAndFeel(UIBase &base) : ui_base_(base) {
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool, bool) override {
            const auto isPressed = button.getToggleState() ^ reverse_;

            auto bounds = button.getLocalBounds().toFloat();
            if (with_shadow_) {
                bounds = ui_base_.drawShadowEllipse(g, bounds, ui_base_.getFontSize() * 0.4f * shrink_scale_, {});
                bounds = ui_base_.drawInnerShadowEllipse(g, bounds, ui_base_.getFontSize() * 0.15f * shrink_scale_,
                                                         {.flip = true});
            } else {
                bounds = ui_base_.getShadowEllipseArea(bounds, ui_base_.getFontSize() * 0.3f * shrink_scale_, {});
                g.setColour(ui_base_.getBackgroundColor());
                g.fillEllipse(bounds);
            }
            if (isPressed) {
                if (with_shadow_) {
                    const auto innerBound = ui_base_.getShadowEllipseArea(bounds, ui_base_.getFontSize() * 0.1f, {});
                    ui_base_.drawInnerShadowEllipse(g, innerBound, ui_base_.getFontSize() * 0.375f, {
                                                        .dark_shadow_color = ui_base_.getDarkShadowColor().
                                                        withMultipliedAlpha(button_depth_),
                                                        .bright_shadow_color = ui_base_.getBrightShadowColor().
                                                        withMultipliedAlpha(button_depth_),
                                                        .change_dark = true,
                                                        .change_bright = true
                                                    });
                }
            }
            if (editable_) {
                if (drawable_ == nullptr) {
                    const auto textBound = button.getLocalBounds().toFloat();
                    if (isPressed) {
                        g.setColour(ui_base_.getTextColor().withAlpha(1.f));
                    } else {
                        g.setColour(ui_base_.getTextColor().withAlpha(0.5f));
                    }
                    g.setFont(ui_base_.getFontSize() * scale_);
                    g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
                } else {
                    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * .5f * scale_;
                    const auto draw_bound = bounds.withSizeKeepingCentre(radius, radius);
                    if (isPressed) {
                        internal_img_->drawWithin(g, draw_bound, juce::RectanglePlacement::Flags::centred, 1.f);
                    } else {
                        internal_img_->drawWithin(g, draw_bound, juce::RectanglePlacement::Flags::centred, .5f);
                    }
                }
            }
        }

        inline void setEditable(const bool f) { editable_ = f; }

        [[nodiscard]] inline float getDepth() const { return button_depth_; }

        inline void setDepth(const float x) { button_depth_ = x; }

        inline void setDrawable(juce::Drawable *x) {
            drawable_ = x;
            updateImages();
        }

        void enableShadow(const bool f) { with_shadow_ = f; }

        void setScale(const float x) { scale_ = x; }

        void setReverse(const bool f) { reverse_ = f; }

        void setShrinkScale(const float x) { shrink_scale_ = x; }

        void updateImages() {
            if (drawable_ != nullptr) {
                internal_img_ = drawable_->createCopy();
                internal_img_->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor());
            }
        }

    private:
        bool editable_{true}, reverse_{false}, with_shadow_{true};
        float button_depth_{0.f}, shrink_scale_{1.f}, scale_{1.f};
        juce::Drawable *drawable_ = nullptr;
        std::unique_ptr<juce::Drawable> internal_img_;

        UIBase &ui_base_;
    };
}
