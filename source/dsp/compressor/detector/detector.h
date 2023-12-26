// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef ZLECOMP_DETECTOR_H
#define ZLECOMP_DETECTOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "iter_funcs.h"

namespace zlCompressor {
    /**
     * a detector that performs attack/release on the input gain
     * @tparam FloatType
     */
    template<typename FloatType>
    class Detector {
    public:
        enum PhaseType {
            gain, level, phaseNUM
        };

        Detector() = default;

        Detector(const Detector<FloatType> &d);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * apply attack/release on the target gain and return the current gain
         * @param target the target gain
         * @return the current gain
         */
        FloatType process(FloatType target);

        inline void setAStyle(IterType idx) { aStyle.store(idx); }

        inline IterType getAStyle() const { return static_cast<IterType>(aStyle.load()); }

        inline void setRStyle(IterType idx) { rStyle.store(idx); }

        inline IterType getRStyle() const { return static_cast<IterType>(rStyle.load()); }

        inline void setAttack(FloatType v) {
            v = juce::jmax(v, FloatType(0.0001));
            attack.store(v);
            aPara.store(juce::jmin(getScale(smooth.load(), aStyle.load()) / v * deltaT.load(), FloatType(0.9)));
        }

        inline FloatType getAttack() const { return attack.load(); }

        inline void setRelease(FloatType v) {
            v = juce::jmax(v, FloatType(0.0001));
            release.store(v);
            rPara.store(juce::jmin(getScale(smooth.load(), rStyle.load()) / v * deltaT.load(), FloatType(0.9)));
        }

        inline FloatType getRelease() const { return release.load(); }

        inline void setSmooth(FloatType v) {
            smooth.store(v);
            setAttack(attack.load());
            setRelease(release.load());
        }

        inline FloatType getSmooth() const { return smooth.load(); }

        inline void setDeltaT(FloatType v) { deltaT.store(v); }

        inline FloatType getDeltaT() const { return deltaT.load(); }

        inline void setPhase(PhaseType idx) { phase.store(idx); }

    private:
        std::atomic<size_t> aStyle, rStyle, phase;
        std::atomic<FloatType> attack, release, aPara, rPara, smooth;
        std::atomic<FloatType> deltaT = FloatType(1) / FloatType(44100);
        FloatType xC = 1.0, xS = 1.0;

        inline static FloatType sgn(FloatType val) {
            return (FloatType(0) < val) - (val < FloatType(0));
        }
    };

} // zldetector

#endif //ZLECOMP_DETECTOR_H
