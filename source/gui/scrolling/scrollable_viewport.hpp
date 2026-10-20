// Copyright (C) 2026 - zsliu98
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
#include "scroll_model.hpp"
#include "styled_scroll_bar.hpp"

namespace zlgui::scrolling {
    class ScrollableViewport final : public juce::Component,
                                     private juce::ScrollBar::Listener {
    public:
        explicit ScrollableViewport(UIBase& base);

        ~ScrollableViewport() override;

        void resized() override;

        void mouseWheelMove(const juce::MouseEvent& event,
                            const juce::MouseWheelDetails& wheel) override;

        bool keyPressed(const juce::KeyPress& key) override;

        void setViewedComponent(juce::Component* component, int content_height);

        void setViewPosition(double position);

        [[nodiscard]] double getViewPosition() const;

        void flushPendingScroll();

    private:
        UIBase& base_;
        juce::Component* viewed_component_{nullptr};
        StyledScrollBar scroll_bar_;
        int content_height_{0};
        ScrollModel scroll_model_;

        [[nodiscard]] juce::Rectangle<int> getContentBounds() const;

        [[nodiscard]] juce::Rectangle<int> getScrollBarBounds() const;

        [[nodiscard]] double getMaximumViewPosition() const;

        [[nodiscard]] bool needsScrollBar() const;

        void requestViewPosition(double position);

        void updateScrollRange();

        void updateViewedComponentBounds();

        void scrollBarMoved(juce::ScrollBar*, double new_range_start) override;
    };
}
