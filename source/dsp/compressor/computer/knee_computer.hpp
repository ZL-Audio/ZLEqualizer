// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZLECOMP_COMPUTER_H
#define ZLECOMP_COMPUTER_H

#include <juce_audio_processors/juce_audio_processors.h>

#include <boost/circular_buffer.hpp>
#include <boost/math/interpolators/cubic_hermite.hpp>

#include "virtual_computer.hpp"

namespace zlCompressor {
    /**
     * a computer that computes the current compression
     * @tparam FloatType
     */
    template<typename FloatType>
    class KneeComputer : VirtualComputer<FloatType> {
    public:
        KneeComputer() { interpolate(); }

        KneeComputer(const KneeComputer<FloatType> &c);

        ~KneeComputer() override;

        FloatType eval(FloatType x);

        /**
         * computes the current compression
         * @param x input level (in dB)
         * @return current compression (in dB)
         */
        FloatType process(FloatType x) override;

        inline void setThreshold(FloatType v) {
            const juce::ScopedLock scopedLock(paraUpdateLock);
            threshold.store(v);
            interpolate();
        }

        inline FloatType getThreshold() const { return threshold.load(); }

        inline void setRatio(FloatType v) {
            const juce::ScopedLock scopedLock(paraUpdateLock);
            ratio.store(v);
            interpolate();
        }

        inline FloatType getRatio() const { return ratio.load(); }

        inline void setKneeW(FloatType v) {
            const juce::ScopedLock scopedLock(paraUpdateLock);
            kneeW.store(v);
            interpolate();
        }

        inline FloatType getKneeW() const { return kneeW.load(); }

        inline void setKneeD(FloatType v) {
            const juce::ScopedLock scopedLock(paraUpdateLock);
            kneeD.store(v);
            interpolate();
        }

        inline FloatType getKneeD() const { return kneeD.load(); }

        inline void setKneeS(FloatType v) {
            const juce::ScopedLock scopedLock(paraUpdateLock);
            kneeS.store(v);
            interpolate();
        }

        inline FloatType getKneeS() const { return kneeS.load(); }

        inline void setBound(FloatType v) {
            bound.store(v);
        }

        inline FloatType getBound() const { return bound.load(); }

        inline FloatType getReductionAtKnee() const {return reductionAtKnee.load(); }

    private:
        std::atomic<FloatType> threshold{0}, ratio{1};
        std::atomic<FloatType> kneeW{FloatType(0.0625)}, kneeD{FloatType(0.5)}, kneeS{FloatType(0.5)};
        std::atomic<FloatType> bound{60};
        std::atomic<FloatType> tempA {0}, tempB{0}, tempC{0};
        std::atomic<FloatType> reductionAtKnee{0};
        std::unique_ptr<boost::math::interpolators::cubic_hermite<std::array<FloatType, 3>>> cubic;

        void interpolate();
    };

} // KneeComputer

#endif //ZLECOMP_COMPUTER_H
