// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "vector_transform/vector_copy.hpp"
#include "vector_transform/vector_add.hpp"
#include "vector_transform/vector_sub.hpp"
#include "vector_transform/vector_multiply.hpp"
#include "vector_transform/vector_clamp.hpp"
#include "vector_transform/vector_mag_to_db.hpp"
#include "vector_transform/vector_flip.hpp"
#include "vector_transform/vector_fma.hpp"

#include "vector_reduce/vector_sum.hpp"
#include "vector_reduce/vector_sum_sqr.hpp"
#include "vector_reduce/vector_dot_product.hpp"
#include "vector_reduce/vector_max_of.hpp"
#include "vector_reduce/vector_max_abs_of.hpp"

namespace zldsp::vector {
    template <typename F>
    using aligned_vector = std::vector<F, hwy::AlignedAllocator<F>>;
}
