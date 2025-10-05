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

namespace zlpanel {
    template <typename FloatType>
    class AtomicBound {
    public:
        AtomicBound() = default;

        void store(const juce::Rectangle<FloatType>& bound,
                   const std::memory_order order = std::memory_order::relaxed) {
            x_.store(bound.getX(), order);
            y_.store(bound.getY(), order);
            width_.store(bound.getWidth(), order);
            height_.store(bound.getHeight(), order);
        }

        juce::Rectangle<FloatType> load(const std::memory_order order = std::memory_order::relaxed) const {
            return {x_.load(order), y_.load(order), width_.load(order), height_.load(order)};
        }

        FloatType getX() const { return x_.load(); }

        FloatType getY() const { return y_.load(); }

        FloatType getWidth() const { return width_.load(); }

        FloatType getHeight() const { return height_.load(); }

    private:
        std::atomic<FloatType> x_{}, y_{}, width_{}, height_{};
    };

    template <typename FloatType>
    class AtomicPoint {
    public:
        AtomicPoint() = default;

        void store(const juce::Point<FloatType>& p, const std::memory_order order = std::memory_order::relaxed) {
            x_.store(p.getX(), order);
            y_.store(p.getY(), order);
        }

        juce::Point<FloatType> load(const std::memory_order order = std::memory_order::relaxed) const {
            return {x_.load(order), y_.load(order)};
        }

        FloatType getX() const { return x_.load(); }

        FloatType getY() const { return y_.load(); }

    private:
        std::atomic<FloatType> x_{}, y_{};
    };
}
