// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Weverything"
#include <kfr/kfr.h>
#include <kfr/dsp.hpp>
#pragma clang diagnostic pop

namespace zlVector {
    template<typename FloatType>
    inline void copy(FloatType *out, const FloatType* in, const size_t size) {
        auto v1 = kfr::make_univector(out, size);
        auto v2 = kfr::make_univector(in, size);
        v1 = v2;
    }

    inline void convert(double *out, const float* in, const size_t size) {
        for (size_t i = 0; i < size; ++i) {
            out[i] = static_cast<double>(in[i]);
        }
    }

    inline void convert(float *out, const double* in, const size_t size) {
        for (size_t i = 0; i < size; ++i) {
            out[i] = static_cast<float>(in[i]);
        }
    }

    template<typename FloatType>
    inline void add(FloatType *in1, const FloatType in2, const size_t size) {
        auto v1 = kfr::make_univector(in1, size);
        auto v2 = kfr::make_univector(in2, size);
        v1 = v1 + v2;
    }

    template<typename FloatType>
    inline void multiply(FloatType *in, const FloatType m, const size_t size) {
        auto v = kfr::make_univector(in, size);
        v = v * m;
    }

    template<typename FloatType>
    inline void multiply(FloatType *in, FloatType *mul, const size_t size) {
        auto v1 = kfr::make_univector(in, size);
        auto v2 = kfr::make_univector(mul, size);
        v1 = v1 * v2;
    }

    template<typename FloatType>
    inline void clamp(FloatType *in, const FloatType lo, const FloatType hi, const size_t size) {
        auto v = kfr::make_univector(in, size);
        v = kfr::clamp(v, lo, hi);
    }

    template<typename FloatType>
    inline FloatType sumsqr(FloatType *in, size_t size) {
        auto v = kfr::make_univector(in, size);
        return kfr::sumsqr(v);
    }
}
