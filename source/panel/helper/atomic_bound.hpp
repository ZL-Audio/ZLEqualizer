// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLPANEL_ATOMIC_BOUND_HPP
#define ZLPANEL_ATOMIC_BOUND_HPP

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlPanel {
    class AtomicBound {
    public:
        AtomicBound() = default;

        void store(const juce::Rectangle<float> &bound) {
            x.store(bound.getX());
            y.store(bound.getY());
            width.store(bound.getWidth());
            height.store(bound.getHeight());
        }

        juce::Rectangle<float> load() const {
            return {x.load(), y.load(), width.load(), height.load()};
        }

        float getX() const { return x.load(); }

        float getY() const { return y.load(); }

        float getWidth() const { return width.load(); }

        float getHeight() const { return height.load(); }

    private:
        std::atomic<float> x{0.f}, y{0.f}, width{0.f}, height{0.f};
    };

    class AtomicPoint{
    public:
        AtomicPoint() = default;

        void store(const juce::Point<float> &p) {
            x.store(p.getX());
            y.store(p.getY());
        }

        juce::Point<float> load() const {
            return {x.load(), y.load()};
        }

        float getX() const { return x.load(); }

        float getY() const { return y.load(); }

    private:
        std::atomic<float> x{0.f}, y{0.f};
    };
}

#endif //ZLPANEL_ATOMIC_BOUND_HPP
