//
// Created by Zishu Liu on 12/29/23.
//

#include "compact_button.h"

namespace zlInterface {
    CompactButton::CompactButton(const juce::String &labelText, UIBase &base) : uiBase(base), lookAndFeel(uiBase),
        animator{std::make_unique<friz::DisplaySyncController>(this)} {
        button.setClickingTogglesState(true);
        button.setButtonText(labelText);
        button.setLookAndFeel(&lookAndFeel);
        button.onClick = [this]() { this->buttonDownAnimation(); };
        addAndMakeVisible(button);
    }

    CompactButton::~CompactButton() {
        button.setLookAndFeel(nullptr);
    }

    void CompactButton::resized() {
        button.setBounds(getLocalBounds());
    }

    void CompactButton::buttonDownAnimation() {
        if (button.getToggleState() && lookAndFeel.getDepth() < 0.1f) {
            if (animator.getAnimation(animationId) != nullptr)
                return;
            auto effect{
                friz::makeAnimation<friz::Parametric, 1>(
                    // ID of the animation
                    animationId, {0.f}, {1.f}, 250, friz::Parametric::kLinear)
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
