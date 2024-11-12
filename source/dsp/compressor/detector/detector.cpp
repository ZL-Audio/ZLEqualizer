// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "detector.hpp"

namespace zlCompressor {

    template<typename FloatType>
    Detector<FloatType>::Detector(const Detector<FloatType> &d) {
        setDeltaT(d.getDeltaT());
        setAStyle(d.getAStyle());
        setRStyle(d.getRStyle());
        setAttack(d.getAttack());
        setRelease(d.getRelease());
        setSmooth(d.getSmooth());
    }

    template<typename FloatType>
    void Detector<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<FloatType>(spec.sampleRate));
        setBufferSize(static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    FloatType Detector<FloatType>::process(FloatType target) {
        bool ra = ((xC < target) == (phase.load() == Detector::gain));
        FloatType para = ra ? rPara.load() : aPara.load();
        size_t style = ra ? rStyle.load() : aStyle.load();
        FloatType distanceS = target - xS;
        FloatType distanceC = xS * smooth.load() + target * (1 - smooth.load()) - xC;
        FloatType slopeS = juce::jmin(para * std::abs(funcs<FloatType>[style](std::abs(distanceS))), std::abs(distanceS));
        FloatType slopeC = juce::jmin(para * std::abs(funcs<FloatType>[style](std::abs(distanceC))), std::abs(target - xC));
        xS += slopeS * sgn(distanceS);
        xC += slopeC * sgn(distanceC);
        xS = juce::jmax(xS, FloatType(1e-5));
        xC = juce::jmax(xC, FloatType(1e-5));
        return xC;
    }

    template<typename FloatType>
    void Detector<FloatType>::reset() {
        xC = 1.0;
        xS = 1.0;
    }

    template
    class Detector<float>;

    template
    class Detector<double>;
} // zldetector