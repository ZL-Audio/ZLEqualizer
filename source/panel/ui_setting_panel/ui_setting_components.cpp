// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_components.hpp"
#include "../helper/paint_selected_card.hpp"

namespace zlpanel {
    UISettingTabBar::UISettingTabBar(zlgui::UIBase& base) : base_(base) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setWantsKeyboardFocus(true);
    }

    void UISettingTabBar::paint(juce::Graphics& g) {
        const auto font_size = base_.getFontSize();
        g.setFont(juce::FontOptions{1.5f * font_size});

        for (auto index = 0; index < static_cast<int>(tab_names_.size()); ++index) {
            const auto selected = index == selected_index_;
            const auto hovered = index == hovered_index_;
            const auto tab_bounds = getTabBounds(index);
            paintSelectableCard(g, tab_bounds, base_.getTextColour(), font_size, selected, hovered);

            g.setColour(base_.getTextColour().withAlpha(selected ? .95f : .62f));
            g.drawFittedText(tab_names_[static_cast<size_t>(index)],
                             tab_bounds.reduced(juce::roundToInt(font_size * .5f), 0),
                             juce::Justification::centred, 1);
        }
    }

    void UISettingTabBar::mouseMove(const juce::MouseEvent& event) {
        const auto next_index = getTabAt(event.getPosition());
        if (hovered_index_ != next_index) {
            hovered_index_ = next_index;
            repaint();
        }
    }

    void UISettingTabBar::mouseExit(const juce::MouseEvent&) {
        if (hovered_index_ >= 0) {
            hovered_index_ = -1;
            repaint();
        }
    }

    void UISettingTabBar::mouseDown(const juce::MouseEvent& event) {
        mouse_down_index_ = event.mods.isPopupMenu() ? -1 : getTabAt(event.getPosition());
        if (mouse_down_index_ >= 0) {
            grabKeyboardFocus();
        }
    }

    void UISettingTabBar::mouseUp(const juce::MouseEvent& event) {
        const auto index = getTabAt(event.getPosition());
        if (!event.mods.isPopupMenu() && event.mouseWasClicked() && index == mouse_down_index_) {
            selectTab(index, true);
        }
        mouse_down_index_ = -1;
    }

    bool UISettingTabBar::keyPressed(const juce::KeyPress& key) {
        if (key == juce::KeyPress::leftKey) {
            selectTab(juce::jmax(0, selected_index_ - 1), true);
            return true;
        }
        if (key == juce::KeyPress::rightKey) {
            selectTab(juce::jmin(static_cast<int>(tab_names_.size()) - 1, selected_index_ + 1), true);
            return true;
        }
        if (key == juce::KeyPress::homeKey) {
            selectTab(0, true);
            return true;
        }
        if (key == juce::KeyPress::endKey) {
            selectTab(static_cast<int>(tab_names_.size()) - 1, true);
            return true;
        }
        return false;
    }

    void UISettingTabBar::setSelectedIndex(const int index) {
        selectTab(index, false);
    }

    juce::Rectangle<int> UISettingTabBar::getTabBounds(const int index) const {
        if (!juce::isPositiveAndBelow(index, static_cast<int>(tab_names_.size()))) {
            return {};
        }
        const auto bounds = getLocalBounds();
        const auto left = bounds.getX() + bounds.getWidth() * index / static_cast<int>(tab_names_.size());
        const auto right = bounds.getX() + bounds.getWidth() * (index + 1) / static_cast<int>(tab_names_.size());
        return {left, bounds.getY(), right - left, bounds.getHeight()};
    }

    int UISettingTabBar::getTabAt(const juce::Point<int> position) const {
        if (!getLocalBounds().contains(position)) {
            return -1;
        }
        for (auto index = 0; index < static_cast<int>(tab_names_.size()); ++index) {
            if (getTabBounds(index).contains(position)) {
                return index;
            }
        }
        return -1;
    }

    void UISettingTabBar::selectTab(const int index, const bool send_notification) {
        if (!juce::isPositiveAndBelow(index, static_cast<int>(tab_names_.size())) ||
            selected_index_ == index) {
            return;
        }
        selected_index_ = index;
        repaint();
        if (send_notification && onTabSelected) {
            onTabSelected(index);
        }
    }
}
