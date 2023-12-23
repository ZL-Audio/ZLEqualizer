// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_MARTIN_COEFFS_H
#define ZLEQUALIZER_MARTIN_COEFFS_H

#include "helpers.h"
#include "analog_func.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlIIR {
    class MartinCoeff {
    public:
        static coeff22 get1LowPass(double w0);

        static coeff22 get1HighPass(double w0);

        static coeff22 get1TiltShelf(double w0, double g);

        static coeff22 get1LowShelf(double w0, double g);

        static coeff22 get1HighShelf(double w0, double g);

        static coeff33 get2LowPass(double w0, double q);

        static coeff33 get2HighPass(double w0, double q);

        static coeff33 get2BandPass(double w0, double q);

        static coeff33 get2Notch(double w0, double q);

        static coeff33 get2Peak(double w0, double g, double q);

        static coeff33 get2TiltShelf(double w0, double g, double q);

        static coeff33 get2LowShelf(double w0, double g, double q);

        static coeff33 get2HighShelf(double w0, double g, double q);

    private:
        static coeff3 solve_a(double w0, double b, double c=1);

        static coeff3 get_AB(coeff3 a);

        static bool check_AB(coeff3 A);

        static coeff3 get_ab(coeff3 A);

        static coeff3 get_phi(double w);

        static coeff3 linear_solve(std::array<coeff3, 3> A, coeff3 b);
    };
}

#endif //ZLEQUALIZER_MARTIN_COEFFS_H
