// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.



#ifndef ZLECOMP_ITER_FUNCS_H
#define ZLECOMP_ITER_FUNCS_H

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlCompressor {
    enum IterType {
        classic, style1, style2, style3, style4, styleNUM
    };

    template<typename FloatType>
    static std::array<std::function<FloatType(FloatType)>, IterType::styleNUM> funcs = {
            [](FloatType x) { return x; },
            [](FloatType x) { return x * (FloatType(0.5) + (FloatType(1.5) - x) * x); },
            [](FloatType x) { return std::sin(x * juce::MathConstants<FloatType>::halfPi); },
            [](FloatType x) { return std::sin(x * juce::MathConstants<FloatType>::halfPi) - x; },
            [](FloatType x) { return x * (1 - x); }
    };

    template<typename FloatType>
    static const std::array<FloatType, IterType::styleNUM> scales0 = {
            FloatType(9.1049 / 2),
            FloatType(15.9207 / 2),
            FloatType(5.9168 / 2),
            FloatType(17.6049 / 2),
            FloatType(11.4074 / 2)
    };

    template<typename FloatType>
    static const std::array<FloatType, IterType::styleNUM> scales1 = {
            FloatType(11.6419 / 2),
            FloatType(20.7267 / 2),
            FloatType(7.5338 / 2),
            FloatType(22.0537 / 2),
            FloatType(14.0136 / 2)
    };

    template<typename FloatType>
    static FloatType getScale(FloatType smooth, size_t style) {
        auto proportion = (1 - smooth) * (1 - smooth);
        return scales0<FloatType>[style] * proportion +
               scales1<FloatType>[style] * (1 - proportion);
    }
}

#endif //ZLECOMP_ITER_FUNCS_H
