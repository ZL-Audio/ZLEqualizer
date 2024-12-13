// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef CLICK_BUTTON_HPP
#define CLICK_BUTTON_HPP

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    class ClickButton final : public juce::Component {
    public:
        explicit ClickButton(UIBase &base,
                             juce::Drawable *normalImage = nullptr,
                             juce::Drawable *normalOnImage = nullptr)
            : uiBase(base), normal(normalImage), normalOn(normalOnImage) {
            if (normalOnImage != nullptr) {
                button.setToggleable(true);
                button.setClickingTogglesState(true);
            }
            updateImages();
            addAndMakeVisible(button);
        }

        ~ClickButton() override = default;

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            const auto width = bound.getWidth(), height = bound.getHeight();
            bound.removeFromLeft(lPadding * width);
            bound.removeFromRight(rPadding * width);
            bound.removeFromTop(uPadding * height);
            bound.removeFromBottom(dPadding * height);
            button.setBounds(bound.toNearestInt());
        }

        inline juce::DrawableButton &getButton() { return button; }

        void lookAndFeelChanged() override {
            updateImages();
            button.setColour(juce::DrawableButton::backgroundColourId, uiBase.getBackgroundColor());
            button.setColour(juce::DrawableButton::backgroundOnColourId, uiBase.getBackgroundColor());
        }

        void updateImages() {
            if (normal != nullptr) {
                normalImg = normal->createCopy();
                overImg = normal->createCopy();
                normalImg->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor().withAlpha(.5f));
                overImg->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
            }
            if (normalOn != nullptr) {
                normalOnImg = normalOn->createCopy();
                overOnImg = normalOnImg->createCopy();
                normalOnImg->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor().withAlpha(.5f));
                overOnImg->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
            }
            button.setImages(normalImg.get(), overImg.get(), nullptr, nullptr,
                             normalOnImg.get(), overOnImg.get(), nullptr, nullptr);
        }

        void setPadding(const float l, const float r, const float u, const float d) {
            lPadding = l;
            rPadding = r;
            uPadding = u;
            dPadding = d;
        }

    private:
        zlInterface::UIBase &uiBase;
        juce::DrawableButton button{"", juce::DrawableButton::ImageFitted};
        juce::Drawable *normal = nullptr, *normalOn = nullptr;

        float lPadding{0.f}, rPadding{0.f}, uPadding{0.f}, dPadding{0.f};
        std::unique_ptr<juce::Drawable> normalImg, normalOnImg, overImg, overOnImg;
    };
} // zlInterface

#endif //CLICK_BUTTON_HPP
