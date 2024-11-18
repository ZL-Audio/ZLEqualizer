// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQMATCH_EQ_MATCH_OPTIMIZER_HPP
#define ZLEQMATCH_EQ_MATCH_OPTIMIZER_HPP
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#include "nlopt.hpp"
#pragma GCC diagnostic pop
#include "../filter/ideal_filter/ideal_filter.hpp"

namespace zlEqMatch {
    template<size_t filterNum>
    class EqMatchOptimizer final {
    public:
        static constexpr double eps = 1e-6;
        std::vector<double> &getDiffs() { return mDiffs; }

        void runSimple() {
            for (size_t i = 0; i < filterNum; i++) {
            }
        }

    private:
        std::array<zlFilter::Empty<double>, filterNum> filters;
        zlFilter::Ideal<double, 1> mFilter;
        std::vector<double> mDiffs;
        std::vector<double> mWs;

        struct optFData {
            zlFilter::FilterType filterType;
            zlFilter::Ideal<double, 1> *filter;
            std::vector<double> *diffs;
            std::vector<double> *ws;
        };

        static double func(const std::vector<double> &x, std::vector<double> &grad, void *f_data) {
            auto *data = static_cast<optFData *>(f_data);
            const auto mse = calculateMSE(data->filterType, x[0], x[1], x[2],
                                          data->filter, data->diffs, data->ws);
            if (!grad.empty()) {
                const auto mse0l = calculateMSE(data->filterType, x[0] - eps, x[1], x[2],
                                                data->filter, data->diffs, data->ws);
                const auto mse0r = calculateMSE(data->filterType, x[0] + eps, x[1], x[2],
                                                data->filter, data->diffs, data->ws);
                grad[0] = (mse0r - mse0l) / (2 * eps);
                const auto mse1l = calculateMSE(data->filterType, x[0], x[1] - eps, x[2],
                                                data->filter, data->diffs, data->ws);
                const auto mse1r = calculateMSE(data->filterType, x[0], x[1] + eps, x[2],
                                                data->filter, data->diffs, data->ws);
                grad[1] = (mse1r - mse1l) / (2 * eps);
                const auto mse2l = calculateMSE(data->filterType, x[0], x[1], x[2] - eps,
                                                data->filter, data->diffs, data->ws);
                const auto mse2r = calculateMSE(data->filterType, x[0], x[1], x[2] + eps,
                                                data->filter, data->diffs, data->ws);
                grad[2] = (mse2r - mse2l) / (2 * eps);
            }
            return mse;
        }

        static double calculateMSE(const zlFilter::FilterType filterType,
                                   const double freq, const double gain, const double q,
                                   zlFilter::Ideal<double, 1> *filter,
                                   const std::vector<double> *diffs, const std::vector<double> *ws) {
            filter->setFilterType(filterType);
            filter->setFreq(freq);
            filter->setGain(gain);
            filter->setQ(q);
            filter->updateMagnitude(*ws);
            const auto &dB = filter->getDBs();
            double mse = 0.0;
            for (size_t i = 0; i < dB.size(); ++i) {
                const auto t = dB[i] - diffs->at(i);
                mse += t * t;
            }
            return mse / static_cast<double>(dB.size());
        }
    };
} // zlEqMatch

#endif //ZLEQMATCH_EQ_MATCH_OPTIMIZER_HPP
