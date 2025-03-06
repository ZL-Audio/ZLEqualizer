// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_MULTILINGUAL_HPP
#define ZL_INTERFACE_MULTILINGUAL_HPP

#include "labels.hpp"
#include "en.hpp"
#include "zh_cn.hpp"

namespace zlInterface::multilingual {
    enum languages {
        lang_system,
        lang_en,
        lang_zh_cn,
        langNum
    };
}

#endif //ZL_INTERFACE_MULTILINGUAL_HPP
