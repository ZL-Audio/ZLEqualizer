// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scrollable_viewport.hpp"

namespace zlgui::scrolling {
    ScrollableViewport::ScrollableViewport(UIBase& base) : base_(base), scroll_bar_(base) {
        scroll_bar_.addListener(this);
        addAndMakeVisible(scroll_bar_);
        setWantsKeyboardFocus(true);
        setOpaque(false);
    }

    ScrollableViewport::~ScrollableViewport() {
        scroll_bar_.removeListener(this);
        if (viewed_component_ != nullptr) {
            removeChildComponent(viewed_component_);
        }
    }

    void ScrollableViewport::resized() {
        scroll_bar_.setBounds(getScrollBarBounds());
        updateScrollRange();
        setViewPosition(scroll_model_.getPosition());
        updateViewedComponentBounds();
    }

    void ScrollableViewport::mouseWheelMove(const juce::MouseEvent&,
                                            const juce::MouseWheelDetails& wheel) {
        if (!needsScrollBar()) {
            return;
        }
        const auto multiplier = wheel.isSmooth ? 4.5 : 3.0;
        requestViewPosition(scroll_model_.getTargetPosition() - static_cast<double>(wheel.deltaY) *
                            static_cast<double>(base_.getFontSize()) * multiplier);
    }

    bool ScrollableViewport::keyPressed(const juce::KeyPress& key) {
        const auto target_position = scroll_model_.getTargetPosition();
        const auto visible_height = static_cast<double>(getContentBounds().getHeight());
        if (key == juce::KeyPress::upKey) {
            requestViewPosition(target_position - base_.getFontSize() * 2.5);
            return true;
        }
        if (key == juce::KeyPress::downKey) {
            requestViewPosition(target_position + base_.getFontSize() * 2.5);
            return true;
        }
        if (key == juce::KeyPress::pageUpKey) {
            requestViewPosition(target_position - visible_height * .85);
            return true;
        }
        if (key == juce::KeyPress::pageDownKey) {
            requestViewPosition(target_position + visible_height * .85);
            return true;
        }
        if (key == juce::KeyPress::homeKey) {
            requestViewPosition(0.0);
            return true;
        }
        if (key == juce::KeyPress::endKey) {
            requestViewPosition(getMaximumViewPosition());
            return true;
        }
        return false;
    }

    void ScrollableViewport::setViewedComponent(juce::Component* component, const int content_height) {
        if (viewed_component_ != component) {
            if (viewed_component_ != nullptr) {
                removeChildComponent(viewed_component_);
            }
            viewed_component_ = component;
            scroll_model_.reset();
            if (viewed_component_ != nullptr) {
                addAndMakeVisible(viewed_component_);
            }
        }
        content_height_ = juce::jmax(0, content_height);
        updateScrollRange();
        setViewPosition(scroll_model_.getTargetPosition());
        updateViewedComponentBounds();
        repaint();
    }

    void ScrollableViewport::setViewPosition(const double position) {
        if (scroll_model_.setPosition(position, getMaximumViewPosition())) {
            scroll_bar_.setCurrentRangeStart(scroll_model_.getPosition(), juce::dontSendNotification);
            updateViewedComponentBounds();
            repaint();
        }
    }

    double ScrollableViewport::getViewPosition() const {
        return scroll_model_.getTargetPosition();
    }

    void ScrollableViewport::flushPendingScroll() {
        if (scroll_model_.flush(getMaximumViewPosition())) {
            scroll_bar_.setCurrentRangeStart(scroll_model_.getPosition(), juce::dontSendNotification);
            updateViewedComponentBounds();
            repaint();
        }
    }

    juce::Rectangle<int> ScrollableViewport::getContentBounds() const {
        const auto inset = juce::roundToInt(base_.getFontSize() * .28f);
        auto bounds = getLocalBounds().reduced(inset);
        if (scroll_bar_.isVisible()) {
            bounds.removeFromRight(scroll_bar_.getWidth() +
                                   StyledScrollBar::getContentGap(base_.getFontSize()));
        }
        return bounds;
    }

    juce::Rectangle<int> ScrollableViewport::getScrollBarBounds() const {
        const auto inset = juce::roundToInt(base_.getFontSize() * .28f);
        auto bounds = getLocalBounds().reduced(inset);
        return bounds.removeFromRight(StyledScrollBar::getThickness(base_.getFontSize()));
    }

    double ScrollableViewport::getMaximumViewPosition() const {
        const auto inset = juce::roundToInt(base_.getFontSize() * .28f);
        const auto visible_height = juce::jmax(0, getHeight() - 2 * inset);
        return juce::jmax(0.0, static_cast<double>(content_height_ - visible_height));
    }

    bool ScrollableViewport::needsScrollBar() const {
        const auto inset = juce::roundToInt(base_.getFontSize() * .28f);
        return content_height_ > juce::jmax(0, getHeight() - 2 * inset);
    }

    void ScrollableViewport::requestViewPosition(const double position) {
        scroll_model_.requestPosition(position, getMaximumViewPosition());
    }

    void ScrollableViewport::updateScrollRange() {
        const auto visible_height = static_cast<double>(juce::jmax(
            0, getHeight() - 2 * juce::roundToInt(base_.getFontSize() * .28f)));
        const auto total_height = static_cast<double>(content_height_);
        const auto needs_scrollbar = total_height > visible_height && visible_height > 0.0;
        scroll_bar_.setVisible(needs_scrollbar);
        scroll_bar_.setRangeLimits(0.0, juce::jmax(total_height, visible_height));
        scroll_bar_.setSingleStepSize(base_.getFontSize());
        scroll_model_.clampToMaximum(juce::jmax(0.0, total_height - visible_height));
        scroll_bar_.setCurrentRange(scroll_model_.getPosition(), visible_height,
                                    juce::dontSendNotification);
    }

    void ScrollableViewport::updateViewedComponentBounds() {
        if (viewed_component_ == nullptr) {
            return;
        }
        const auto content_bounds = getContentBounds();
        viewed_component_->setBounds(content_bounds.getX(),
                                     content_bounds.getY() - juce::roundToInt(scroll_model_.getPosition()),
                                     content_bounds.getWidth(), content_height_);
    }

    void ScrollableViewport::scrollBarMoved(juce::ScrollBar*, const double new_range_start) {
        requestViewPosition(new_range_start);
    }
}
