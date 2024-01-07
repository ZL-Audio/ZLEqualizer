// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_button.hpp"

namespace zlInterface {
    CompactButton::CompactButton(const juce::String &labelText, UIBase &base) : uiBase(base), lookAndFeel(uiBase),
        animator{} {
        setBufferedToImage(true);
        button.setClickingTogglesState(true);
        button.setButtonText(labelText);
        button.setLookAndFeel(&lookAndFeel);
        button.onClick = [this]() { this->buttonDownAnimation(); };
        addAndMakeVisible(button);

        setEditable(true);
    }

    CompactButton::~CompactButton() {
        button.setLookAndFeel(nullptr);
    }

    void CompactButton::resized() {
        if (fit.load()) {
            button.setBounds(getLocalBounds());
        } else {
            auto bound = getLocalBounds().toFloat();
            const auto radius = juce::jmin(bound.getHeight(), bound.getWidth());
            bound = bound.withSizeKeepingCentre(radius, radius);
            button.setBounds(bound.toNearestInt());
        }
    }

    void CompactButton::buttonDownAnimation() {
        if (button.getToggleState() && lookAndFeel.getDepth() < 0.1f) {
            if (animator.getAnimation(animationId) != nullptr)
                return;
            auto effect{
                friz::makeAnimation<friz::Parametric, 1>(
                    // ID of the animation
                    animationId, {0.f}, {1.f}, animationDuration, friz::Parametric::kLinear)
            };
            effect->updateFn = [this](int id, const auto &vals) {
                juce::ignoreUnused(id);
                lookAndFeel.setDepth(vals[0]);
                repaint();
            };

            // pass the animation object to the animator, which will start running it immediately.
            animator.addAnimation(std::move(effect));
        } else if (!button.getToggleState()) {
            lookAndFeel.setDepth(0.f);
            animator.cancelAnimation(animationId, false);
            repaint();
        }
    }
}
