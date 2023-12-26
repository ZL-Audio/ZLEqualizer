// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 12/19/23.
//

#ifndef ZLEQUALIZER_SINGLE_FILTER_H
#define ZLEQUALIZER_SINGLE_FILTER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "coeff/design_filter.h"

namespace zlIIR {
    /** stl does not support constexpr log/pow, np.logspace(1, np.log10(20000), 200) */
    constexpr std::array<double, 200> frequencies = {1.00000000e+01, 1.03893431e+01, 1.07938451e+01, 1.12140960e+01,
                                                     1.16507092e+01, 1.21043215e+01, 1.25755950e+01, 1.30652172e+01,
                                                     1.35739024e+01, 1.41023930e+01, 1.46514600e+01, 1.52219046e+01,
                                                     1.58145590e+01, 1.64302880e+01, 1.70699900e+01, 1.77345983e+01,
                                                     1.84250827e+01, 1.91424507e+01, 1.98877489e+01, 2.06620647e+01,
                                                     2.14665281e+01, 2.23023126e+01, 2.31706379e+01, 2.40727707e+01,
                                                     2.50100276e+01, 2.59837758e+01, 2.69954363e+01, 2.80464851e+01,
                                                     2.91384558e+01, 3.02729416e+01, 3.14515978e+01, 3.26761442e+01,
                                                     3.39483674e+01, 3.52701238e+01, 3.66433419e+01, 3.80700253e+01,
                                                     3.95522556e+01, 4.10921955e+01, 4.26920920e+01, 4.43542793e+01,
                                                     4.60811827e+01, 4.78753220e+01, 4.97393148e+01, 5.16758809e+01,
                                                     5.36878459e+01, 5.57781453e+01, 5.79498291e+01, 6.02060660e+01,
                                                     6.25501479e+01, 6.49854950e+01, 6.75156606e+01, 7.01443366e+01,
                                                     7.28753582e+01, 7.57127103e+01, 7.86605327e+01, 8.17231266e+01,
                                                     8.49049605e+01, 8.82106769e+01, 9.16450991e+01, 9.52132381e+01,
                                                     9.89203002e+01, 1.02771694e+02, 1.06773040e+02, 1.10930175e+02,
                                                     1.15249165e+02, 1.19736312e+02, 1.24398163e+02, 1.29241521e+02,
                                                     1.34273450e+02, 1.39501295e+02, 1.44932682e+02, 1.50575537e+02,
                                                     1.56438092e+02, 1.62528902e+02, 1.68856853e+02, 1.75431179e+02,
                                                     1.82261472e+02, 1.89357697e+02, 1.96730209e+02, 2.04389765e+02,
                                                     2.12347540e+02, 2.20615146e+02, 2.29204645e+02, 2.38128571e+02,
                                                     2.47399944e+02, 2.57032291e+02, 2.67039667e+02, 2.77436673e+02,
                                                     2.88238479e+02, 2.99460847e+02, 3.11120149e+02, 3.23233399e+02,
                                                     3.35818270e+02, 3.48893124e+02, 3.62477038e+02, 3.76589833e+02,
                                                     3.91252100e+02, 4.06485232e+02, 4.22311456e+02, 4.38753862e+02,
                                                     4.55836443e+02, 4.73584122e+02, 4.92022795e+02, 5.11179365e+02,
                                                     5.31081783e+02, 5.51759088e+02, 5.73241450e+02, 5.95560212e+02,
                                                     6.18747941e+02, 6.42838467e+02, 6.67866942e+02, 6.93869883e+02,
                                                     7.20885231e+02, 7.48952403e+02, 7.78112351e+02, 8.08407622e+02,
                                                     8.39882418e+02, 8.72582664e+02, 9.06556071e+02, 9.41852210e+02,
                                                     9.78522580e+02, 1.01662069e+03, 1.05620211e+03, 1.09732462e+03,
                                                     1.14004820e+03, 1.18443520e+03, 1.23055037e+03, 1.27846100e+03,
                                                     1.32823700e+03, 1.37995100e+03, 1.43367845e+03, 1.48949773e+03,
                                                     1.54749030e+03, 1.60774078e+03, 1.67033706e+03, 1.73537049e+03,
                                                     1.80293595e+03, 1.87313202e+03, 1.94606114e+03, 2.02182969e+03,
                                                     2.10054824e+03, 2.18233165e+03, 2.26729923e+03, 2.35557497e+03,
                                                     2.44728767e+03, 2.54257114e+03, 2.64156440e+03, 2.74441190e+03,
                                                     2.85126369e+03, 2.96227569e+03, 3.07760986e+03, 3.19743449e+03,
                                                     3.32192441e+03, 3.45126125e+03, 3.58563374e+03, 3.72523793e+03,
                                                     3.87027752e+03, 4.02096412e+03, 4.17751760e+03, 4.34016638e+03,
                                                     4.50914778e+03, 4.68470836e+03, 4.86710426e+03, 5.05660163e+03,
                                                     5.25347694e+03, 5.45801747e+03, 5.67052163e+03, 5.89129950e+03,
                                                     6.12067321e+03, 6.35897742e+03, 6.60655984e+03, 6.86378172e+03,
                                                     7.13101835e+03, 7.40865966e+03, 7.69711074e+03, 7.99679247e+03,
                                                     8.30814210e+03, 8.63161391e+03, 8.96767988e+03, 9.31683034e+03,
                                                     9.67957474e+03, 1.00564423e+04, 1.04479830e+04, 1.08547681e+04,
                                                     1.12773910e+04, 1.17164685e+04, 1.21726412e+04, 1.26465746e+04,
                                                     1.31389603e+04, 1.36505167e+04, 1.41819902e+04, 1.47341563e+04,
                                                     1.53078206e+04, 1.59038200e+04, 1.65230244e+04, 1.71663370e+04,
                                                     1.78346965e+04, 1.85290782e+04, 1.92504952e+04, 2.00000000e+04};

    /**
     * a static IIR filter
     * @tparam FloatType
     */
    template<typename FloatType>
    class Filter {
    public:
        Filter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void setFreq(FloatType x);

        inline FloatType getFreq() { return static_cast<FloatType>(freq.load()); }

        void setGain(FloatType x);

        inline FloatType getGain() { return static_cast<FloatType>(gain.load()); }

        void setQ(FloatType x);

        inline FloatType getQ() { return static_cast<FloatType>(q.load()); }

        void setFilterType(FilterType x);

        inline FilterType getFilterType() { return filterType.load(); }

        void setOrder(size_t x);

        inline size_t getOrder() { return order.load(); }

        void addDBs(std::array<FloatType, frequencies.size()> &x, FloatType scale=1.0);

    private:
        std::vector<juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<FloatType>, juce::dsp::IIR::Coefficients<FloatType>>> filters;
        std::atomic<double> freq = 1000, gain = 0, q = 0.707;
        std::atomic<size_t> order = 2;
        std::atomic<FilterType> filterType = FilterType::peak;
        juce::dsp::ProcessSpec processSpec{48000, 512, 2};

        std::array<FloatType, frequencies.size()> dBs{};
        juce::ReadWriteLock magLock;

        void updateParas();

        void updateDBs();
    };
}

#endif //ZLEQUALIZER_SINGLE_FILTER_H
