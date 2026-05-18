// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../highway_import.hpp"

namespace zldsp::vector {
    namespace hn = hwy::HWY_NAMESPACE;

    template <typename F>
    HWY_INLINE void copy(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const size_t size) {
        std::memcpy(out, in, sizeof(F) * size);
    }

    template <typename F>
    HWY_INLINE void copy(std::span<F*> outs, std::span<F*> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<F>(outs[i], ins[i], size);
        }
    }

    template <typename F1, typename F2>
    HWY_INLINE void copy(F1* HWY_RESTRICT out, const F2* HWY_RESTRICT in, const size_t size) {
        std::copy(in, in + size, out);
    }

    template <typename F1, typename F2>
    HWY_INLINE void copy(std::span<F1*> outs, std::span<F2*> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<F1, F2>(outs[i], ins[i], size);
        }
    }
}
