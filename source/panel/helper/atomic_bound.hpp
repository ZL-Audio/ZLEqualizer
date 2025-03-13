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

namespace zlPanel {
    template <typename FloatType>
    class AtomicBound {
    public:
        AtomicBound() = default;

        void store(const juce::Rectangle<FloatType> &bound) {
            x.store(bound.getX());
            y.store(bound.getY());
            width.store(bound.getWidth());
            height.store(bound.getHeight());
        }

        juce::Rectangle<FloatType> load() const {
            return {x.load(), y.load(), width.load(), height.load()};
        }

        FloatType getX() const { return x.load(); }

        FloatType getY() const { return y.load(); }

        FloatType getWidth() const { return width.load(); }

        FloatType getHeight() const { return height.load(); }

    private:
        std::atomic<FloatType> x{}, y{}, width{}, height{};
    };

    template <typename FloatType>
    class AtomicPoint {
    public:
        AtomicPoint() = default;

        void store(const juce::Point<FloatType> &p) {
            x.store(p.getX());
            y.store(p.getY());
        }

        juce::Point<FloatType> load() const {
            return {x.load(), y.load()};
        }

        FloatType getX() const { return x.load(); }

        FloatType getY() const { return y.load(); }

    private:
        std::atomic<FloatType> x{}, y{};
    };
}
