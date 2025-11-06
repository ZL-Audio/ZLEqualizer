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
#include "../interface_definitions.hpp"

namespace zlgui {
    class ResizeCorner final : public juce::ResizableCornerComponent {
    public:
        static constexpr auto kDefaultRectScale = 0.33f;

        enum ScaleType {
            kScaleWithWidth,
            kScaleWithHeight,
            kScaleWithFontSize
        };

        ResizeCorner(UIBase& base,
                     juce::Component* componentToResize,
                     juce::ComponentBoundsConstrainer* constrainer,
                     const ScaleType scale_type = kScaleWithWidth,
                     const float corner_scale = .025f)
            : juce::ResizableCornerComponent(componentToResize, constrainer),
              base_(base),
              corner_scale_(corner_scale),
              scale_type_(scale_type) {
            setAlpha(.1f);
            setBufferedToImage(true);
        }

        void paint(juce::Graphics& g) override {
            const auto bound = getLocalBounds().toFloat();
            auto rect1 = juce::Rectangle<float>{
                bound.getX() + bound.getWidth() * (1.f - kDefaultRectScale), bound.getY(),
                bound.getWidth() * kDefaultRectScale, bound.getHeight()
            };
            auto rect2 = juce::Rectangle<float>{
                bound.getX(), bound.getY() + bound.getHeight() * (1.f - kDefaultRectScale),
                bound.getWidth(), bound.getHeight() * kDefaultRectScale
            };

            g.setColour(base_.getBackgroundColour());
            g.fillRect(rect1);
            g.fillRect(rect2);

            rect1.reduce(bound.getWidth() * 0.05f, bound.getHeight() * 0.05f);
            rect2.reduce(bound.getWidth() * 0.05f, bound.getHeight() * 0.05f);

            g.setColour(base_.getTextColour());
            g.fillRect(rect1);
            g.fillRect(rect2);
        }

        void mouseEnter(const juce::MouseEvent&) override {
            setAlpha(1.f);
        }

        void mouseExit(const juce::MouseEvent&) override {
            setAlpha(.1f);
        }

        void resized() override {
            updateCornerBound();
        }

        void moved() override {
            updateCornerBound();
        }

    private:
        UIBase& base_;
        const float corner_scale_ = 0.05f;
        const ScaleType scale_type_ = kScaleWithWidth;

        void updateCornerBound() {
            auto* parent = getParentComponent();
            if (parent != nullptr) {
                const auto parent_bound = parent->getLocalBounds();
                int corner_size{0};
                switch (scale_type_) {
                case kScaleWithWidth: {
                    corner_size = parent_bound.getWidth();
                    break;
                }
                case kScaleWithHeight: {
                    corner_size = parent_bound.getHeight();
                    break;
                }
                case kScaleWithFontSize: {
                    const auto max_font_size = static_cast<float>(parent_bound.getWidth()) * 0.016f;
                    const auto min_font_size = max_font_size * .25f;
                    const auto font_size = base_.getFontMode() == 0
                        ? max_font_size * base_.getFontScale()
                        : std::clamp(base_.getStaticFontSize(), min_font_size, max_font_size);
                    corner_size = static_cast<int>(std::round(font_size));
                }
                }
                corner_size = std::max(1, static_cast<int>(
                                           std::round(static_cast<float>(corner_size) * corner_scale_)));
                const auto corner_bound = juce::Rectangle<int>(
                    parent_bound.getWidth() - corner_size,
                    parent_bound.getHeight() - corner_size,
                    corner_size, corner_size);
                if (corner_bound != getBoundsInParent()) {
                    setBounds(corner_bound);
                }
            }
        }
    };
}
