// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zldsp::analyzer {
    enum class MagType {
        kPeak, kRMS
    };

    enum class StereoType {
        kStereo, kLeft, kRight, kMid, kSide
    };

    inline constexpr float kSqrt2Over2 = static_cast<float>(
            0.7071067811865475244008443621048490392848359376884740365883398690);
}
