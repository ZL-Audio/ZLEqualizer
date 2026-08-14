// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../gui/interface_definitions.hpp"

namespace zlpanel {
    class WarningOverlay final : public juce::Component {
    public:
        explicit WarningOverlay(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void mouseMove(const juce::MouseEvent& event) override;

        void mouseExit(const juce::MouseEvent&) override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        bool keyPressed(const juce::KeyPress& key) override;

        void show(const juce::String& message, const juce::String& action, bool destructive,
                  std::function<void()> on_confirm);

        void showMessage(const juce::String& message);

        void hide();

    private:
        enum class Target {
            none,
            cancel,
            confirm
        };

        zlgui::UIBase& base_;
        juce::String message_;
        juce::String action_;
        std::function<void()> on_confirm_;
        Target hovered_{Target::none};
        Target pressed_{Target::none};
        bool destructive_{false};
        bool show_cancel_{true};

        juce::Rectangle<int> getCardBounds() const;

        juce::Rectangle<int> getCancelBounds() const;

        juce::Rectangle<int> getConfirmBounds() const;

        juce::Rectangle<int> getMessageBounds() const;

        Target getTarget(juce::Point<int> position) const;

        void paintButton(juce::Graphics& g, juce::Rectangle<int> bounds,
                         const juce::String& text, Target target, bool destructive) const;

        void confirm();
    };
}
