// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

namespace zldsp::interpolation {
    /**
     * modified Akima spline interpolation with increasing input/output X
     * @tparam FloatType the float type of input/output
     */
    template<typename FloatType>
    class SeqMakima {
    public:
        /**
         *
         * @param x input X pointer
         * @param y input Y pointer
         * @param point_num number of input points
         * @param left_derivative left derivative
         * @param right_derivative right derivative
         */
        explicit SeqMakima(FloatType *x, FloatType *y, size_t point_num,
                           FloatType left_derivative, FloatType right_derivative)
            : xs_(x), ys_(y), input_size_(point_num),
              left_derivative_(left_derivative), right_derivative_(right_derivative) {
            derivatives_.resize(point_num);
            deltas_.resize(point_num - 1);
        }

        /**
         * call this to update derivatives if input has been updated
         */
        void prepare() {
            for (size_t i = 0; i < deltas_.size(); ++i) {
                deltas_[i] = (ys_[i + 1] - ys_[i]) / (xs_[i + 1] - xs_[i]);
            }
            auto left_delta = FloatType(2) * deltas_[0] - deltas_[1];
            auto right_delta = FloatType(2) * deltas_.end()[-1] - deltas_.end()[-2];

            derivatives_.front() = left_derivative_;
            derivatives_.back() = right_derivative_;

            derivatives_[1] = calculateD(left_delta, deltas_[0], deltas_[1], deltas_[2]);

            for (size_t i = 2; i < derivatives_.size() - 2; ++i) {
                derivatives_[i] = calculateD(deltas_[i - 2], deltas_[i - 1], deltas_[i], deltas_[i + 1]);
            }

            derivatives_.end()[-2] = calculateD(deltas_.end()[-3], deltas_.end()[-2], deltas_.end()[-1], right_delta);
        }

        /**
         * evaluate the spline at output X
         * @param x output X pointer
         * @param y output Y pointer
         * @param point_num number of output points
         */
        void eval(FloatType *x, FloatType *y, const size_t point_num) {
            size_t current_pos = 0;
            size_t start_idx = 0, end_idx = point_num - 1;
            while (start_idx <= end_idx && x[start_idx] <= xs_[0]) {
                y[start_idx] = ys_[0];
                start_idx += 1;
            }
            while (end_idx > start_idx && x[end_idx] >= xs_[input_size_ - 1]) {
                y[end_idx] = ys_[input_size_ - 1];
                end_idx -= 1;
            }
            for (size_t i = start_idx; i <= end_idx; ++i) {
                while (current_pos + 2 < input_size_ && x[i] >= xs_[current_pos + 1]) {
                    current_pos += 1;
                }
                const auto t = (x[i] - xs_[current_pos]) / (xs_[current_pos + 1] - xs_[current_pos]);
                y[i] = h00(t) * ys_[current_pos] +
                       h10(t) * (xs_[current_pos + 1] - xs_[current_pos]) * derivatives_[current_pos] +
                       h01(t) * ys_[current_pos + 1] +
                       h11(t) * (xs_[current_pos + 1] - xs_[current_pos]) * derivatives_[current_pos + 1];
            }
        }

    private:
        FloatType *xs_, *ys_;
        size_t input_size_;
        std::vector<FloatType> derivatives_, deltas_;
        FloatType left_derivative_, right_derivative_;

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
