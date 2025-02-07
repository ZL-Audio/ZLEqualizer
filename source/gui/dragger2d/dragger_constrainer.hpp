// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

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
                const auto _minOffLeft = getMinimumWhenOffTheLeft();
                const auto _minOffRight = getMinimumWhenOffTheRight();
                if (_minOffLeft > 0) {
                    const int limit = limits.getX() + juce::jmin(_minOffLeft - bounds.getWidth(), 0);
                    if (bounds.getX() < limit) {
                        pos.setX(limit);
                    }
                }
                if (_minOffRight > 0) {
                    const auto limit = limits.getRight() - juce::jmin(_minOffRight, bounds.getWidth());
                    if (bounds.getX() > limit) {
                        pos.setX(limit);
                    }
                }
            }

            if (!enableY.load()) {
                pos.setY(previousPos.getY());
            } else {
                const auto _minOffTop = getMinimumWhenOffTheTop();
                const auto _minOffBottom = getMinimumWhenOffTheBottom();
                if (_minOffTop > 0) {
                    const auto limit = limits.getY() + juce::jmin(_minOffTop - bounds.getHeight(), 0);
                    if (bounds.getY() < limit) {
                        pos.setY(limit);
                    }
                }
                if (_minOffBottom > 0) {
                    const auto limit = limits.getBottom() - juce::jmin(_minOffBottom, bounds.getHeight());
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
