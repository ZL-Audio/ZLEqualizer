// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_DRAGGER_CONSTRAINER_HPP
#define ZLEqualizer_DRAGGER_CONSTRAINER_HPP

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    class DraggerConstrainer : public juce::ComponentBoundsConstrainer {
    public:
        void checkBounds(juce::Rectangle<int> &bounds,
                         const juce::Rectangle<int> &previousBounds,
                         const juce::Rectangle<int> &limits,
                         bool isStretchingTop, bool isStretchingLeft,
                         bool isStretchingBottom, bool isStretchingRight) override {
            juce::ignoreUnused(limits, isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);
            auto pos = bounds.getPosition();
            const auto previousPos = previousBounds.getPosition();
            if (!enableX.load()) {
                pos.setX(previousPos.getX());
            } else {
                const auto minOffLeft = getMinimumWhenOffTheLeft();
                const auto minOffRight = getMinimumWhenOffTheRight();
                if (minOffLeft > 0) {
                    const int limit = limits.getX() + juce::jmin(minOffLeft - bounds.getWidth(), 0);
                    if (bounds.getX() < limit) {
                        pos.setX(limit);
                    }
                }
                if (minOffRight > 0) {
                    const auto limit = limits.getRight() - juce::jmin(minOffRight, bounds.getWidth());
                    if (bounds.getX() > limit) {
                        pos.setX(limit);
                    }
                }
            }

            if (!enableY.load()) {
                pos.setY(previousPos.getY());
            } else {
                const auto minOffTop = getMinimumWhenOffTheTop();
                const auto minOffBottom = getMinimumWhenOffTheBottom();
                if (minOffTop > 0) {
                    const auto limit = limits.getY() + juce::jmin(minOffTop - bounds.getHeight(), 0);
                    if (bounds.getY() < limit) {
                        pos.setY(limit);
                    }
                }
                if (minOffBottom > 0) {
                    const auto limit = limits.getBottom() - juce::jmin(minOffBottom, bounds.getHeight());
                    if (bounds.getY() > limit) {
                        pos.setY(limit);
                    }
                }
            }

            bounds.setPosition(pos);
        }

        void setXON(const bool f) { enableX.store(f); }

        void setYON(const bool f) { enableY.store(f); }

    private:
        std::atomic<bool> enableX{true}, enableY{true};
    };
}

#endif //ZLEqualizer_DRAGGER_CONSTRAINER_HPP
