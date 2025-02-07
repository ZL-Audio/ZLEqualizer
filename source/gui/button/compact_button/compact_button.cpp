// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "compact_button.hpp"

namespace zlInterface {
    CompactButton::CompactButton(const juce::String &labelText, UIBase &base) : uiBase(base), lookAndFeel(uiBase),
        animator{} {
        button.setClickingTogglesState(true);
        button.setButtonText(labelText);
        button.setLookAndFeel(&lookAndFeel);
        button.onClick = [this]() { this->buttonDownAnimation(); };
        addAndMakeVisible(button);

        setEditable(true);
    }

    CompactButton::~CompactButton() {
        animator.cancelAllAnimations(false);
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
            auto frizEffect{
                friz::makeAnimation<friz::Parametric, static_cast<int>(1)>(
                    // ID of the animation
                    animationId, {0.f}, {1.f}, animationDuration, friz::Parametric::kLinear)
            };
            frizEffect->updateFn = [this](int id, const auto &vals) {
                juce::ignoreUnused(id);
                lookAndFeel.setDepth(vals[0]);
                repaint();
            };

            // pass the animation object to the animator, which will start running it immediately.
            animator.addAnimation(std::move(frizEffect));
        } else if (!button.getToggleState()) {
            lookAndFeel.setDepth(0.f);
            animator.cancelAnimation(animationId, false);
            repaint();
        }
    }
}
