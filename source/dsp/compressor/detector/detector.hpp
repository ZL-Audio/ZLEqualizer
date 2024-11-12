// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLECOMP_DETECTOR_H
#define ZLECOMP_DETECTOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "iter_funcs.hpp"

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
            attack.store(v);
            v = juce::jmax(FloatType(0.001) * v, FloatType(0.0001));
            aPara.store(juce::jmin(getScale(smooth.load(), aStyle.load()) / v * deltaT.load(), FloatType(0.9)));
        }

        inline FloatType getAttack() const { return attack.load(); }

        inline void setRelease(FloatType v) {
            release.store(v);
            v = juce::jmax(FloatType(0.001) * v, FloatType(0.0001));
            rPara.store(juce::jmin(getScale(smooth.load(), rStyle.load()) / v * deltaT.load(), FloatType(0.9)));
        }

        inline FloatType getRelease() const { return release.load(); }

        inline void setSmooth(const FloatType v) {
            smooth.store(v);
            setAttack(attack.load());
            setRelease(release.load());
        }

        inline FloatType getSmooth() const { return smooth.load(); }

        inline void setDeltaT(const FloatType v) { deltaT.store(v); }

        inline FloatType getDeltaT() const { return deltaT.load(); }

        inline void setPhase(const PhaseType idx) { phase.store(idx); }

        inline void setBufferSize(const int x) {
            if (x != bufferSize.load()) {
                bufferSize.store(x);
                deltaT.store(static_cast<FloatType>(x) / sampleRate.load());
                setAttack(getAttack());
                setRelease(getRelease());
            }
        }

        inline int getBufferSize() const {
            return bufferSize.load();
        }

    private:
        std::atomic<size_t> aStyle, rStyle, phase;
        std::atomic<FloatType> attack, release, aPara, rPara, smooth{0.f};
        std::atomic<int> bufferSize{0};
        std::atomic<FloatType> deltaT = FloatType(1) / FloatType(44100), sampleRate{48000};
        FloatType xC = 1.0, xS = 1.0;

        inline static FloatType sgn(FloatType val) {
            return static_cast<FloatType>(FloatType(0) < val) - static_cast<FloatType>(val < FloatType(0));
        }
    };
} // zldetector

#endif //ZLECOMP_DETECTOR_H
