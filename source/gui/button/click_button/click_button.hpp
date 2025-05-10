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

namespace zlgui {
    class ClickButton final : public juce::Component {
    public:
        explicit ClickButton(UIBase &base,
                             juce::Drawable *normalImage = nullptr,
                             juce::Drawable *normalOnImage = nullptr,
                             const multilingual::Labels labelIdx = multilingual::Labels::kLabelNum)
            : ui_base_(base), normal_(normalImage), normal_on_(normalOnImage) {
            if (normalOnImage != nullptr) {
                button_.setToggleable(true);
                button_.setClickingTogglesState(true);
            }
            button_.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::white.withAlpha(0.f));
            button_.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::white.withAlpha(0.f));
            addAndMakeVisible(button_);

            if (labelIdx != multilingual::Labels::kLabelNum) {
                button_.setTooltip(ui_base_.getToolTipText(labelIdx));
            }
        }

        ~ClickButton() override = default;

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            const auto width = bound.getWidth(), height = bound.getHeight();
            bound.removeFromLeft(l_pad_ * width);
            bound.removeFromRight(r_pad_ * width);
            bound.removeFromTop(u_pad_ * height);
            bound.removeFromBottom(d_pad_ * height);
            button_.setBounds(bound.toNearestInt());
        }

        inline juce::DrawableButton &getButton() { return button_; }

        void lookAndFeelChanged() override {
            updateImages();
        }

        void visibilityChanged() override {
            updateImages();
        }

        void updateImages() {
            if (normal_ != nullptr) {
                normal_img_ = normal_->createCopy();
                over_img_ = normal_->createCopy();
                normal_img_->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor().withAlpha(.5f));
                over_img_->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor());
            }
            if (normal_on_ != nullptr) {
                normal_on_img_ = normal_on_->createCopy();
                over_on_img_ = normal_on_img_->createCopy();
                normal_on_img_->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor().withAlpha(.5f));
                over_on_img_->replaceColour(juce::Colour(0, 0, 0), ui_base_.getTextColor());
            }
            button_.setImages(normal_img_.get(), over_img_.get(), nullptr, nullptr,
                             normal_on_img_.get(), over_on_img_.get(), nullptr, nullptr);
        }

        void setPadding(const float l, const float r, const float u, const float d) {
            l_pad_ = l;
            r_pad_ = r;
            u_pad_ = u;
            d_pad_ = d;
        }

    private:
        zlgui::UIBase &ui_base_;
        juce::DrawableButton button_{"", juce::DrawableButton::ImageFitted};
        juce::Drawable *normal_ = nullptr, *normal_on_ = nullptr;

        float l_pad_{0.f}, r_pad_{0.f}, u_pad_{0.f}, d_pad_{0.f};
        std::unique_ptr<juce::Drawable> normal_img_, normal_on_img_, over_img_, over_on_img_;
    };
} // zlgui
