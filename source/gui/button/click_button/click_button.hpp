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

namespace zlgui::button {
    class ClickButton final : public juce::Component {
    public:
        explicit ClickButton(UIBase& base,
                             juce::Drawable* normal_image = nullptr,
                             juce::Drawable* normal_on_image = nullptr,
                             const juce::String& tooltip_text = "")
            : base_(base), normal_(normal_image), normal_on_(normal_on_image) {
            if (normal_on_image != nullptr) {
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

        ~ClickButton() override = default;

        void resized() override {
            button_.setBounds(getLocalBounds());
        }

        inline juce::DrawableButton& getButton() { return button_; }

        void updateImages() {
            if (normal_ != nullptr) {
                normal_img_ = normal_->createCopy();
                over_img_ = normal_->createCopy();
                normal_img_->replaceColour(juce::Colour(0, 0, 0), base_.getTextColour().withAlpha(alpha_));
                over_img_->replaceColour(juce::Colour(0, 0, 0), base_.getTextColour().withAlpha(over_alpha_));
            }
            if (normal_on_ != nullptr) {
                normal_on_img_ = normal_on_->createCopy();
                over_on_img_ = normal_on_->createCopy();
                normal_on_img_->replaceColour(juce::Colour(0, 0, 0), base_.getTextColour().withAlpha(on_alpha_));
                over_on_img_->replaceColour(juce::Colour(0, 0, 0), base_.getTextColour().withAlpha(on_over_alpha_));
            }
            button_.setImages(normal_img_.get(), over_img_.get(), nullptr, nullptr,
                              normal_on_img_.get(), over_on_img_.get(), nullptr, nullptr);
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
        juce::Drawable *normal_ = nullptr, *normal_on_ = nullptr;
        float alpha_{.5f}, over_alpha_{1.f}, on_alpha_{1.f}, on_over_alpha_{1.f};

        std::unique_ptr<juce::Drawable> normal_img_, normal_on_img_, over_img_, over_on_img_;

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
