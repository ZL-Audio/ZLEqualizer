// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <span>

namespace zldsp::filter {
    class IvantsovCoeff {
    public:
        static std::array<double, 4> get1LowPass(double w0);
        static std::array<double, 4> get1HighPass(double w0);

        static std::array<double, 4> get1TiltShelf(double w0, double g);
        static std::array<double, 4> get1LowShelf(double w0, double g);
        static std::array<double, 4> get1HighShelf(double w0, double g);

        static std::array<double, 6> get2LowPass(double w0, double q);
        static std::array<double, 6> get2HighPass(double w0, double q);
        static std::array<double, 6> get2BandPass(double w0, double q);
        static std::array<double, 6> get2Notch(double w0, double q);

        static std::array<double, 6> get2Peak(double w0, double g, double q);
        static std::array<double, 6> get2TiltShelf(double w0, double g, double q);
        static std::array<double, 6> get2LowShelf(double w0, double g, double q);
        static std::array<double, 6> get2HighShelf(double w0, double g, double q);

        static void update2PeakDynamicCache(double w0, double q, std::span<double> cache);
        static std::array<double, 6> get2Peak(double g, std::span<double> cache);

        static void update2TiltShelfDynamicCache(double w0, double q, std::span<double> cache);
        static std::array<double, 6> get2TiltShelf(double g, std::span<double> cache);

        static void update2LowShelfDynamicCache(double w0, double q, std::span<double> cache);
        static std::array<double, 6> get2LowShelf(double g, std::span<double> cache);

        static void update2HighShelfDynamicCache(double w0, double q, std::span<double> cache);
        static std::array<double, 6> get2HighShelf(double g, std::span<double> cache);
    };
}
