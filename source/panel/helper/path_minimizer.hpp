// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLPANEL_PATH_MINIMIZER_HPP
#define ZLPANEL_PATH_MINIMIZER_HPP

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlPanel {
    class PathMinimizer {
    public:
        static constexpr float tol = 0.01f;

        explicit PathMinimizer(juce::Path &path) : p(path) {
        }

        template<bool start=true>
        void startNewSubPath(const float x, const float y) {
            if (start) {
                p.startNewSubPath(x, y);
            } else {
                p.lineTo(x, y);
            }
            startX = x;
            startY = y;
            currentX = x;
            currentY = y;
        }

        void lineTo(const float x, const float y) {
            const auto w = (currentX - startX) / (x - startX);
            const auto linPred = w * startY + (1.f - w) * y;
            if (std::abs(linPred - currentY) > tol) {
                p.lineTo(currentX, currentY);
                startX = x;
                startY = y;
            }
            currentX = x;
            currentY = y;
        }

        void finish() const {
            p.lineTo(currentX, currentY);
        }

    private:
        juce::Path &p;
        float startX{0.}, startY{0.};
        float currentX{0.}, currentY{0.};
    };
}
#endif //ZLPANEL_PATH_MINIMIZER_HPP
