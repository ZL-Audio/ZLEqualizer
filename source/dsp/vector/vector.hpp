// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include "kfr_import.hpp"

namespace zldsp::vector {
    template<typename FloatType>
    inline void copy(FloatType *out, const FloatType* in, const size_t size) {
        std::memcpy(out, in, sizeof(FloatType) * size);
    }

    template<typename FloatType>
    inline void copy(std::span<FloatType *> outs, std::span<FloatType *> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<FloatType>(outs[i], ins[i], size);
        }
    }

    template<typename FloatType1, typename FloatType2>
    inline void copy(FloatType1 *out, const FloatType2* in, const size_t size) {
        for (size_t i = 0; i < size; ++i) {
            out[i] = static_cast<FloatType1>(in[i]);
        }
    }

    template<typename FloatType1, typename FloatType2>
    inline void copy(std::span<FloatType1 *> outs, std::span<FloatType2 *> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<FloatType1, FloatType2>(outs[i], ins[i], size);
        }
    }

    template<typename FloatType>
    inline void multiply(FloatType *in, const FloatType m, const size_t size) {
        auto v = kfr::make_univector(in, size);
        v = v * m;
    }

    template<typename FloatType>
    inline void multiply(FloatType *out, FloatType *in, const FloatType m, const size_t size) {
        auto out_v = kfr::make_univector(out, size);
        auto in_v = kfr::make_univector(in, size);
        out_v = in_v * m;
    }

    template<typename FloatType>
    inline void multiply(FloatType *in, FloatType *mul, const size_t size) {
        auto v1 = kfr::make_univector(in, size);
        auto v2 = kfr::make_univector(mul, size);
        v1 = v1 * v2;
    }

    template<typename FloatType>
    inline void multiply(FloatType *out, FloatType *in, FloatType *mul, const size_t size) {
        auto out_v = kfr::make_univector(out, size);
        auto v1 = kfr::make_univector(in, size);
        auto v2 = kfr::make_univector(mul, size);
        out_v = v1 * v2;
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
