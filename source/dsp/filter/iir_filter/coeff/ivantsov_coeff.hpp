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
        static std::array<double, 3> get1LowPass(double w0);

        static std::array<double, 3> get1HighPass(double w0);

        static std::array<double, 3> get1TiltShelf(double w0, double g);

        static std::array<double, 3> get1AllPass(double w0);

        static std::array<double, 3> get1LowShelf(double w0, double g);

        static std::array<double, 3> get1HighShelf(double w0, double g);

        static void update1ShelfDynamicCache(double w0, double* cache);

        static std::array<double, 3> get1TiltShelfWithCache(double g_linear_sqrt, const double* cache);

        static std::array<double, 3> get1LowShelfWithCache(double g_linear_sqrt, const double* cache);

        static std::array<double, 3> get1HighShelfWithCache(double g_linear_sqrt, const double* cache);

        static std::array<double, 5> get2LowPass(double w0, double q);

        static std::array<double, 5> get2HighPass(double w0, double q);

        static std::array<double, 5> get2BandPass(double w0, double q);

        static std::array<double, 5> get2AllPass(double w0, double q);

        static std::array<double, 5> get2Notch(double w0, double q);

        static std::array<double, 5> get2Peak(double w0, double g, double q);

        static std::array<double, 5> get2TiltShelf(double w0, double g, double q);

        static std::array<double, 5> get2LowShelf(double w0, double g, double q);

        static std::array<double, 5> get2HighShelf(double w0, double g, double q);

        static void update2PeakDynamicCache(double w0, double q, double* cache);

        static std::array<double, 5> get2PeakWithCache(double g_linear_sqrt, const double* cache);

        static void update2ShelfDynamicCache(double w0, double q, double* cache);

        static std::array<double, 5> get2TiltShelfWithCache(double g_linear_sqrt, const double* cache);

        static std::array<double, 5> get2LowShelfWithCache(double g_linear_sqrt, const double* cache);

        static std::array<double, 5> get2HighShelfWithCache(double g_linear_sqrt, const double* cache);
    };
}
