// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#include "knee_computer.hpp"

namespace zlCompressor {
    template<typename FloatType>
    KneeComputer<FloatType>::KneeComputer(const KneeComputer<FloatType> &c) {
        setThreshold(c.getThreshold());
        setRatio(c.getRatio());
        setKneeW(c.getKneeW());
        setKneeD(c.getKneeD());
        setKneeS(c.getKneeS());
        setBound(c.getBound());
    }

    template<typename FloatType>
    KneeComputer<FloatType>::~KneeComputer() = default;

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::eval(FloatType x) {
        const juce::ScopedLock scopedLock(paraUpdateLock);
        if (x <= threshold - kneeW) {
            return x;
        } else if (x >= threshold + kneeW) {
            return juce::jlimit(x - bound.load(), x + bound.load(), x / ratio + (1 - 1 / ratio) * threshold);
        } else {
            return juce::jlimit(x - bound.load(), x + bound.load(), cubic->operator()(x));
        }
    }

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::process(FloatType x) {
        return eval(x) - x;
    }

    template<typename FloatType>
    void KneeComputer<FloatType>::interpolate() {
//        logger.logMessage(juce::String(threshold.load()) + " " + juce::String(ratio.load()));
        std::array initialX{threshold.load() - kneeW.load(),
                            threshold.load(),
                            threshold.load() + kneeW.load()};
        std::array initialY{threshold.load() - kneeW.load(),
                            threshold.load() - kneeD.load() * FloatType(0.75) * kneeW.load() *
                                               (FloatType(1) - FloatType(0.5) / ratio.load() - FloatType(0.5)),
                            threshold.load() + kneeW.load() / ratio.load()};
        std::array initialYX{FloatType(1),
                             kneeS.load() + (FloatType(1) - kneeS.load()) / ratio.load(),
                             FloatType(1) / ratio.load()};
        cubic = std::make_unique<boost::math::interpolators::cubic_hermite<std::array<FloatType, 3>>>(
                std::move(initialX),
                std::move(initialY),
                std::move(initialYX));
        reductionAtKnee.store(process(threshold.load() + kneeW.load()));
    }

    template
    class KneeComputer<float>;

    template
    class KneeComputer<double>;
} // KneeComputer