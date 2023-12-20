// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/17/23.
//

#ifndef ZLEQUALIZER_DESIGN_FILTER_H
#define ZLEQUALIZER_DESIGN_FILTER_H

#include "helpers.h"
#include "martin_coeff.h"

namespace zlIIR {
    class DesignFilter {
    public:
        static std::vector<coeff33> getCoeff(FilterType filterType,
                                             double f, double fs, double gDB, double q, size_t n);

    private:
        static std::vector<coeff33> getLowPass(size_t n, double w0, double q);

        static std::vector<coeff33> getHighPass(size_t n, double w0, double q);

        static std::vector<coeff33> getTiltShelf(size_t n, double w0, double g, double q);

        static std::vector<coeff33> getLowShelf(size_t n, double w0, double g, double q);

        static std::vector<coeff33> getHighShelf(size_t n, double w0, double g, double q);

        static std::vector<coeff33> getBandPass(size_t n, double w0, double q);

        static std::vector<coeff33> getNotch(size_t n, double w0, double q);

        static std::vector<coeff33> getPeak(double w0, double g, double q);

        static std::vector<coeff33> getBandShelf(size_t n, double w0, double g, double q);

        static std::vector<double> getQs(size_t n, double q0);
    };
}

#endif //ZLEQUALIZER_DESIGN_FILTER_H
