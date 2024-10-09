// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERPOLATION_SEQ_MAKIMA_HPP
#define ZL_INTERPOLATION_SEQ_MAKIMA_HPP

#include <vector>

namespace zlInterpolation {
    template<typename FloatType>
    class SeqMakima {
    public:
        explicit SeqMakima(FloatType *x, FloatType *y, size_t pointNum,
                           FloatType leftDerivative, FloatType rightDerivative)
            : xs(x), ys(y), inputSize(pointNum),
              mLeftDerivative(leftDerivative), mRightDerivative(rightDerivative) {
            derivatives.resize(pointNum);
            deltas.resize(pointNum - 1);
        }

        void prepare() {
            for (size_t i = 0; i < deltas.size(); ++i) {
                deltas[i] = (ys[i + 1] - ys[i]) / (xs[i + 1] - xs[i]);
            }
            auto leftDelta = FloatType(2) * deltas[0] - deltas[1];
            auto rightDelta = FloatType(2) * deltas.end()[-1] - deltas.end()[-2];

            derivatives.front() = mLeftDerivative;
            derivatives.back() = mRightDerivative;

            derivatives[1] = calculateD(leftDelta, deltas[0], deltas[1], deltas[2]);

            for (size_t i = 2; i < derivatives.size() - 2; ++i) {
                derivatives[i] = calculateD(deltas[i - 2], deltas[i - 1], deltas[i], deltas[i + 1]);
            }

            derivatives.end()[-2] = calculateD(deltas.end()[-3], deltas.end()[-2], deltas.end()[-1], rightDelta);
        }

        void eval(FloatType *x, FloatType *y, const size_t pointNum) {
            size_t currentPos = 0;
            size_t startIdx = 0, endIdx = pointNum - 1;
            while (startIdx <= endIdx && x[startIdx] <= xs[0]) {
                y[startIdx] = ys[0];
                startIdx += 1;
            }
            while (endIdx > startIdx && x[endIdx] >= xs[inputSize - 1]) {
                y[endIdx] = ys[inputSize - 1];
                endIdx -= 1;
            }
            for (size_t i = startIdx; i <= endIdx; ++i) {
                if (currentPos + 2 < inputSize) {
                    while (currentPos + 2 < inputSize && x[i] >= xs[currentPos + 1]) {
                        currentPos += 1;
                    }
                }
                const auto t = (x[i] - xs[currentPos]) / (xs[currentPos + 1] - xs[currentPos]);
                y[i] = h00(t) * ys[currentPos] +
                       h10(t) * (xs[currentPos + 1] - xs[currentPos]) * derivatives[currentPos] +
                       h01(t) * ys[currentPos + 1] +
                       h11(t) * (xs[currentPos + 1] - xs[currentPos]) * derivatives[currentPos + 1];
            }
        }

    private:
        FloatType *xs, *ys;
        size_t inputSize;
        std::vector<FloatType> derivatives, deltas;
        FloatType mLeftDerivative, mRightDerivative;

        static FloatType h00(FloatType t) {
            return (FloatType(1) + FloatType(2) * t) * (FloatType(1) - t) * (FloatType(1) - t);
        }

        static FloatType h10(FloatType t) {
            return t * (FloatType(1) - t) * (FloatType(1) - t);
        }

        static FloatType h01(FloatType t) {
            return t * t * (FloatType(3) - FloatType(2) * t);
        }

        static FloatType h11(FloatType t) {
            return t * t * (t - FloatType(1));
        }

        static FloatType calculateD(FloatType &delta0, FloatType &delta1, FloatType &delta2, FloatType &delta3) {
            const auto w1 = std::abs(delta3 - delta2) + std::abs(delta3 + delta2) * FloatType(0.5);
            const auto w2 = std::abs(delta1 - delta0) + std::abs(delta1 + delta0) * FloatType(0.5);
            const auto w = w1 / (w1 + w2);
            return w * delta1 + (FloatType(1) - w) * delta2;
        }
    };
}

#endif //ZL_INTERPOLATION_SEQ_MAKIMA_HPP
