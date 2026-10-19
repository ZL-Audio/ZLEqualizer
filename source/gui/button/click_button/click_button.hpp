// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <utility>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlgui::button {
    class ClickButton final : public juce::Component {
    public:
        explicit ClickButton(UIBase& base,
                             std::unique_ptr<juce::Drawable> normal_image = nullptr,
                             std::unique_ptr<juce::Drawable> normal_on_image = nullptr,
                             const juce::String& tooltip_text = "")
            : base_(base), normal_(std::move(normal_image)), normal_on_(std::move(normal_on_image)) {
            if (normal_on_ != nullptr) {
                button_.setToggleable(true);
                button_.setClickingTogglesState(true);
            }
            button_.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::white.withAlpha(0.f));
            button_.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::white.withAlpha(0.f));
            addAndMakeVisible(button_);

            if (tooltip_text.length() > 0) {
                button_.setTooltip(tooltip_text);
            }

            setInterceptsMouseClicks(false, true);
        }

        // use the same drawable for both toggle states
        ClickButton(UIBase& base, std::unique_ptr<juce::Drawable> image,
                    const bool toggleable, const juce::String& tooltip_text = "")
            : ClickButton(base, std::move(image), nullptr, tooltip_text) {
            if (toggleable && normal_ != nullptr) {
                use_normal_for_on_ = true;
                button_.setToggleable(true);
                button_.setClickingTogglesState(true);
            }
        }

        ~ClickButton() override = default;

        void resized() override {
            button_.setEdgeIndent(static_cast<int>(std::round(base_.getFontSize() * .315f)));
            button_.setBounds(getLocalBounds());
        }

        inline juce::DrawableButton& getButton() { return button_; }

        void updateImages() {
            std::unique_ptr<juce::Drawable> normal_img, normal_on_img, over_img, over_on_img;
            if (normal_ != nullptr) {
                const auto normal_colour = base_.getTextColour().withAlpha(alpha_);
                const auto over_colour = base_.getTextColour().withAlpha(over_alpha_);
                normal_img = normal_->createCopy();
                normal_img->replaceColour(juce::Colour(0, 0, 0), normal_colour);
                if (over_colour != normal_colour) {
                    over_img = normal_->createCopy();
                    over_img->replaceColour(juce::Colour(0, 0, 0), over_colour);
                }
            }
            const auto* on_source = use_normal_for_on_ ? normal_.get() : normal_on_.get();
            if (on_source != nullptr) {
                const auto normal_on_colour = base_.getTextColour().withAlpha(on_alpha_);
                const auto over_on_colour = base_.getTextColour().withAlpha(on_over_alpha_);
                normal_on_img = on_source->createCopy();
                normal_on_img->replaceColour(juce::Colour(0, 0, 0), normal_on_colour);
                if (over_on_colour != normal_on_colour) {
                    over_on_img = on_source->createCopy();
                    over_on_img->replaceColour(juce::Colour(0, 0, 0), over_on_colour);
                }
            }
            // the button keeps its own copies of these temporary render images
            button_.setImages(normal_img.get(), over_img.get(), nullptr, nullptr,
                              normal_on_img.get(), over_on_img.get(), nullptr, nullptr);
        }

        void setImageAlpha(const float alpha, const float over_alpha,
                           const float on_alpha = 1.f, const float on_over_alpha = 1.f) {
            alpha_ = alpha;
            over_alpha_ = over_alpha;
            on_alpha_ = on_alpha;
            on_over_alpha_ = on_over_alpha;
        }

        bool getToggleState() const {
            return button_.getToggleState();
        }

    private:
        zlgui::UIBase& base_;
        juce::DrawableButton button_{"", juce::DrawableButton::ImageFitted};
        std::unique_ptr<juce::Drawable> normal_, normal_on_;
        bool use_normal_for_on_{false};
        float alpha_{.5f}, over_alpha_{1.f}, on_alpha_{1.f}, on_over_alpha_{1.f};

        void lookAndFeelChanged() override {
            updateImages();
        }

        void visibilityChanged() override {
            if (isVisible()) {
                updateImages();
            }
        }
    };
}
