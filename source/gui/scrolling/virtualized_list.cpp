// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "virtualized_list.hpp"

#include <cmath>

namespace zlgui::scrolling {
    VirtualizedList::VirtualizedList(UIBase& base, const float content_inset_scale) :
        base_(base), scroll_bar_(base),
        content_inset_scale_(juce::jmax(0.f, content_inset_scale)) {
        row_height_ = juce::roundToInt(base_.getFontSize() * 1.9f);
        scroll_bar_.addListener(this);
        addAndMakeVisible(scroll_bar_);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setOpaque(false);
    }

    VirtualizedList::~VirtualizedList() {
        scroll_bar_.removeListener(this);
    }

    void VirtualizedList::paint(juce::Graphics& g) {
        const auto content = getContentBounds();
        if (row_count_ <= 0 || content.isEmpty()) {
            return;
        }

        const auto scroll_position = scroll_model_.getPosition();
        const auto first_row = juce::jlimit(0, row_count_ - 1,
                                            static_cast<int>(std::floor(scroll_position /
                                                                        static_cast<double>(row_height_))));
        const auto last_row = juce::jlimit(first_row, row_count_ - 1,
                                           static_cast<int>(std::ceil((scroll_position + content.getHeight()) /
                                                                      static_cast<double>(row_height_))));
        const auto y_offset = content.getY() - juce::roundToInt(scroll_position);

        juce::Graphics::ScopedSaveState state{g};
        g.reduceClipRegion(content);
        for (auto row = first_row; row <= last_row; ++row) {
            const auto row_bounds = juce::Rectangle<int>{content.getX(), y_offset + row * row_height_,
                                                         content.getWidth(), row_height_};
            paintRow(g, row, row_bounds, row == selected_row_, row == hovered_row_);
        }
    }

    void VirtualizedList::resized() {
        const auto thickness = StyledScrollBar::getThickness(base_.getFontSize());
        scroll_bar_.setBounds(getLocalBounds().removeFromRight(thickness));
        updateScrollRange();
    }

    void VirtualizedList::mouseMove(const juce::MouseEvent& event) {
        const auto row = getRowAt(event.getPosition());
        if (hovered_row_ != row) {
            hovered_row_ = row;
            repaint();
        }
    }

    void VirtualizedList::mouseExit(const juce::MouseEvent&) {
        if (hovered_row_ >= 0) {
            hovered_row_ = -1;
            repaint();
        }
    }

    void VirtualizedList::mouseDown(const juce::MouseEvent& event) {
        mouse_down_row_ = event.mods.isPopupMenu() ? -1 : getRowAt(event.getPosition());
    }

    void VirtualizedList::mouseUp(const juce::MouseEvent& event) {
        const auto row = getRowAt(event.getPosition());
        if (!event.mods.isPopupMenu() && event.getNumberOfClicks() == 1 && event.mouseWasClicked() &&
            row >= 0 && row == mouse_down_row_) {
            rowClicked(row);
        }
        mouse_down_row_ = -1;
    }

    void VirtualizedList::mouseDoubleClick(const juce::MouseEvent& event) {
        const auto row = getRowAt(event.getPosition());
        if (!event.mods.isPopupMenu() && row >= 0) {
            rowDoubleClicked(row);
        }
    }

    void VirtualizedList::mouseWheelMove(const juce::MouseEvent&,
                                         const juce::MouseWheelDetails& wheel) {
        if (getMaximumScrollPosition() <= 0.0) {
            return;
        }
        const auto multiplier = wheel.isSmooth ? 4.5 : 3.0;
        requestScrollPosition(scroll_model_.getTargetPosition() - static_cast<double>(wheel.deltaY) *
                              static_cast<double>(row_height_) * multiplier);
    }

    void VirtualizedList::lookAndFeelChanged() {
        scroll_bar_.repaint();
        repaint();
    }

    void VirtualizedList::setRowCount(const int row_count) {
        row_count_ = juce::jmax(0, row_count);
        selected_row_ = juce::isPositiveAndBelow(selected_row_, row_count_) ? selected_row_ : -1;
        hovered_row_ = juce::isPositiveAndBelow(hovered_row_, row_count_) ? hovered_row_ : -1;
        updateScrollRange();
        repaint();
    }

    void VirtualizedList::setRowHeight(const int row_height) {
        row_height_ = juce::jmax(1, row_height);
        updateScrollRange();
        repaint();
    }

    void VirtualizedList::setSelectedRow(const int row) {
        const auto next_row = juce::isPositiveAndBelow(row, row_count_) ? row : -1;
        if (selected_row_ != next_row) {
            selected_row_ = next_row;
            repaint();
        }
    }

    void VirtualizedList::scrollToRow(const int row) {
        if (!juce::isPositiveAndBelow(row, row_count_)) {
            return;
        }
        const auto visible_height = getContentBounds().getHeight();
        const auto row_top = row * row_height_;
        const auto row_bottom = row_top + row_height_;
        if (row_top < scroll_model_.getPosition()) {
            setScrollPosition(static_cast<double>(row_top));
        } else if (row_bottom > scroll_model_.getPosition() + visible_height) {
            setScrollPosition(static_cast<double>(row_bottom - visible_height));
        }
    }

    void VirtualizedList::flushPendingScroll() {
        if (scroll_model_.flush(getMaximumScrollPosition())) {
            scroll_bar_.setCurrentRangeStart(scroll_model_.getPosition(), juce::dontSendNotification);
            repaint();
        }
    }

    UIBase& VirtualizedList::getBase() const {
        return base_;
    }

    int VirtualizedList::getContentInset() const {
        return juce::roundToInt(base_.getFontSize() * content_inset_scale_);
    }

    juce::Rectangle<int> VirtualizedList::getContentBounds() const {
        const auto font_size = base_.getFontSize();
        auto bounds = getLocalBounds().reduced(getContentInset());
        if (scroll_bar_.isVisible()) {
            bounds.removeFromRight(scroll_bar_.getWidth() + StyledScrollBar::getContentGap(font_size));
        }
        return bounds;
    }

    int VirtualizedList::getRowAt(const juce::Point<int> position) const {
        const auto content = getContentBounds();
        if (!content.contains(position) || row_count_ <= 0) {
            return -1;
        }
        const auto row = static_cast<int>((static_cast<double>(position.y - content.getY()) +
                                           scroll_model_.getPosition()) /
                                          static_cast<double>(row_height_));
        return juce::isPositiveAndBelow(row, row_count_) ? row : -1;
    }

    double VirtualizedList::getMaximumScrollPosition() const {
        return juce::jmax(0.0, static_cast<double>(row_count_) * static_cast<double>(row_height_) -
                               static_cast<double>(getContentBounds().getHeight()));
    }

    void VirtualizedList::setScrollPosition(const double position) {
        if (scroll_model_.setPosition(position, getMaximumScrollPosition())) {
            scroll_bar_.setCurrentRangeStart(scroll_model_.getPosition(), juce::dontSendNotification);
            repaint();
        }
    }

    void VirtualizedList::requestScrollPosition(const double position) {
        scroll_model_.requestPosition(position, getMaximumScrollPosition());
    }

    void VirtualizedList::updateScrollRange() {
        const auto total_height = static_cast<double>(row_count_) * static_cast<double>(row_height_);
        const auto visible_height = static_cast<double>(juce::jmax(
            0, getLocalBounds().reduced(getContentInset()).getHeight()));
        const auto needs_scrollbar = total_height > visible_height && visible_height > 0.0;
        scroll_bar_.setVisible(needs_scrollbar);
        scroll_bar_.setRangeLimits(0.0, juce::jmax(total_height, visible_height));
        scroll_bar_.setSingleStepSize(row_height_);
        scroll_model_.clampToMaximum(juce::jmax(0.0, total_height - visible_height));
        scroll_bar_.setCurrentRange(scroll_model_.getPosition(), visible_height, juce::dontSendNotification);
    }

    void VirtualizedList::scrollBarMoved(juce::ScrollBar*, const double new_range_start) {
        requestScrollPosition(new_range_start);
    }
}
