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

#ifndef ZLEQUALIZER_DESIGN_FILTER_HPP
#define ZLEQUALIZER_DESIGN_FILTER_HPP

#include "helpers.hpp"
#include "martin_coeff.hpp"

namespace zlIIR {
    class DesignFilter {
    public:
        /**
         * get a vector of 2nd order filter coeffs
         * @param filterType filter type
         * @param f frequency
         * @param fs sample rate
         * @param gDB gain
         * @param q Q
         * @param n filter order
         * @return a vector of 2nd order filter coeffs
         */
        static std::vector<coeff33> getCoeff(FilterType filterType,
                                             double f, double fs, double gDB, double q, size_t n);

        /**
         * update an array of 2nd order filter coeffs
         * @param filterType filter type
         * @param f frequency
         * @param fs sample rate
         * @param gDB gain
         * @param q Q
         * @param n filter order
         * @param coeffs the array of coeffs
         * @return the actual filter size
         */
        static size_t updateCoeff(FilterType filterType,
                                  double f, double fs, double gDB, double q, size_t n, std::array<coeff33, 16> &coeffs);

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

        static size_t updateLowPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateHighPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateTiltShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateLowShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateHighShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateBandPass(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updateNotch(size_t n, double w0, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

        static size_t updatePeak(double w0, double g, double q, std::array<coeff33, 16> &coeffs);

        static size_t updateBandShelf(size_t n, double w0, double g, double q, std::array<coeff33, 16> &coeffs, size_t startIdx);

    };
}

#endif //ZLEQUALIZER_DESIGN_FILTER_HPP
