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
        explicit CompactButtonLookAndFeel(UIBase& base) : base_(base) {
        }

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) override {
            const auto is_pressed = button.getToggleState() ^ reverse_;

            auto bounds = button.getLocalBounds().toFloat();
            if (with_shadow_) {
                bounds = base_.drawShadowEllipse(g, bounds, base_.getFontSize() * 0.4f * shrink_scale_, {});
                bounds = base_.drawInnerShadowEllipse(g, bounds, base_.getFontSize() * 0.15f * shrink_scale_,
                                                      {.flip = true});
            } else {
                bounds = base_.getShadowEllipseArea(bounds, base_.getFontSize() * 0.3f * shrink_scale_, {});
                g.setColour(base_.getBackgroundColour());
                g.fillEllipse(bounds);
            }
            if (is_pressed) {
                if (with_shadow_) {
                    const auto innerBound = base_.getShadowEllipseArea(bounds, base_.getFontSize() * 0.1f, {});
                    base_.drawInnerShadowEllipse(g, innerBound, base_.getFontSize() * 0.375f, {
                                                     .dark_shadow_color = base_.getDarkShadowColour().
                                                     withMultipliedAlpha(button_depth_),
                                                     .bright_shadow_color = base_.getBrightShadowColour().
                                                     withMultipliedAlpha(button_depth_),
                                                     .change_dark = true,
                                                     .change_bright = true
                                                 });
                }
            }
            if (drawable_ == nullptr) {
                const auto textBound = button.getLocalBounds().toFloat();
                if (is_pressed) {
                    g.setColour(base_.getTextColour().withAlpha(1.f));
                } else {
                    g.setColour(base_.getTextColour().withAlpha(0.5f));
                }
                g.setFont(base_.getFontSize() * scale_);
                g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
            } else {
                const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * .5f * scale_;
                const auto draw_bound = bounds.withSizeKeepingCentre(radius, radius);
                if (is_pressed) {
                    internal_img_->drawWithin(g, draw_bound, kPlacement, 1.f);
                } else {
                    internal_img_->drawWithin(g, draw_bound, kPlacement, .5f);
                }
            }
        }

        [[nodiscard]] inline float getDepth() const { return button_depth_; }

        inline void setDepth(const float x) { button_depth_ = x; }

        inline void setDrawable(juce::Drawable* x) {
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
                internal_img_->replaceColour(juce::Colour(0, 0, 0), base_.getTextColour());
            }
        }

    private:
        static constexpr auto kPlacement = juce::RectanglePlacement::centred;

        UIBase& base_;

        bool reverse_{false}, with_shadow_{true};
        float button_depth_{0.f}, shrink_scale_{1.f}, scale_{1.f};
        juce::Drawable* drawable_ = nullptr;
        std::unique_ptr<juce::Drawable> internal_img_;
    };
}
