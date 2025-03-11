// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include "nlopt.hpp"
#pragma GCC diagnostic pop
#include "../filter/ideal_filter/ideal_filter.hpp"

namespace zlEqMatch {
    template<size_t FilterNum>
    class EqMatchOptimizer final : public juce::Thread::Listener {
    public:
        static constexpr double eps = 1e-3, highOrderEps = .1;
        static constexpr double diffMinFreqLog = 2.302585092994046, diffMaxFreqLog = 9.998797732340453;
        static constexpr double minFreqLog = 2.3026, maxFreqLog = 9.9034;
        static constexpr double minGain = -29.999, maxGain = 29.999, gainScale = .15;
        static constexpr double minGainScale = minGain * gainScale, maxGainScale = maxGain * gainScale;
        static constexpr double minQLog = -2.3025850929940455, maxQLog = 2.302585092994046;
        static constexpr std::array<double, 3> lowerBound{minFreqLog, minGain * gainScale, minQLog};
        static constexpr std::array<double, 3> upperBound{maxFreqLog, maxGain * gainScale, maxQLog};
        static constexpr std::array algos1{
            nlopt::algorithm::LD_MMA, nlopt::algorithm::LD_SLSQP, nlopt::algorithm::LD_VAR2
        };
        static constexpr std::array algos2{
            nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::LD_SLSQP
        };
        static constexpr std::array<zlFilter::FilterType, 3> filterTypes{
            zlFilter::FilterType::lowShelf, zlFilter::FilterType::peak, zlFilter::FilterType::highShelf
        };
        static constexpr size_t maximumOrder = 6;
        static constexpr std::array<double, 3> initSol{6.214608098422191, 0.0, -0.3465735902799726};

        EqMatchOptimizer() {
            mFilter.prepare(48000.0);
        }

        std::vector<double> &getDiffs() { return mDiffs; }

        void setDiffs(const double *diffs, const size_t diffsSize) {
            mFilter.prepare(48000.0);
            mFilter.prepareDBSize(diffsSize);
            const auto deltaLog = (diffMaxFreqLog - diffMinFreqLog) / (static_cast<double>(diffsSize) - 1.0);
            auto currentLog = diffMinFreqLog;
            mWs.resize(diffsSize);
            for (size_t i = 0; i < diffsSize; i++) {
                mWs[i] = std::exp(currentLog) / 48000.0 * 2.0 * 3.141592653589793;
                currentLog += deltaLog;
            }
            mDiffs.resize(diffsSize);
            for (size_t i = 0; i < diffsSize; i++) {
                mDiffs[i] = diffs[i];
            }
        }

        void runDeterministic(size_t startIdx = 0, size_t endIdx = 10000) {
            shouldExit.store(false);
            endIdx = std::min(endIdx, mDiffs.size() - 1);
            startIdx = std::min(startIdx, endIdx);
            std::vector<nlopt::algorithm> mAlgos;
            mAlgos.resize(algos1.size());
            std::copy(algos1.begin(), algos1.end(), mAlgos.begin());
            for (size_t i = 0; i < FilterNum; i++) {
                std::array<double, filterTypes.size()> mse{};
                std::array<std::vector<double>, filterTypes.size()> sols{};
                for (size_t j = 0; j < filterTypes.size(); j++) {
                    mFilter.setFilterType(filterTypes[j]);
                    sols[j].resize(initSol.size());
                    std::copy(initSol.begin(), initSol.end(), sols[j].begin());
                    mse[j] = improveSolution(sols[j], mAlgos, startIdx, endIdx);
                }
                const auto idx = static_cast<size_t>(std::min_element(mse.begin(), mse.end()) - mse.begin());
                mseS[idx] = mse[idx];
                filters[i].setFilterType(filterTypes[idx]);
                filters[i].setFreq(std::exp(sols[idx][0]));
                filters[i].setGain(sols[idx][1] / gainScale);
                filters[i].setQ(std::exp(sols[idx][2]));
                updateDiff(filters[i]);
                // if mse is already small enough, exit
                if (mseS[i] < eps) {
                    for (size_t j = i + 1; j < filters.size(); j++) {
                        mseS[j] = mseS[i];
                        filters[j].setFilterType(zlFilter::FilterType::peak);
                        filters[j].setFreq(500.);
                        filters[j].setGain(0.);
                        filters[j].setQ(0.707);
                    }
                    return;
                }
            }
        }

        void runStochastic(const size_t startIdx = 0, const size_t endIdx = 10000) {
            runStochasticPlus({2}, startIdx, endIdx);
        }

        void runStochasticPlus(std::vector<size_t> orders, size_t startIdx = 0, size_t endIdx = 10000) {
            shouldExit.store(false);
            endIdx = std::min(endIdx, mDiffs.size());
            startIdx = std::min(startIdx, endIdx);
            std::vector<nlopt::algorithm> mAlgos1, mAlgos2;
            mAlgos1.resize(algos1.size());
            std::copy(algos1.begin(), algos1.end(), mAlgos1.begin());
            mAlgos2.resize(algos2.size());
            std::copy(algos2.begin(), algos2.end(), mAlgos2.begin());
            // fit filter one by one
            for (size_t i = 0; i < FilterNum; i++) {
                std::vector<double> mseByOrders{};
                std::vector<zlFilter::FilterType> filterTypeByOrders{};
                std::vector<std::vector<double> > solsByOrders{};
                // for all orders
                for (const size_t order: orders) {
                    mFilter.setOrder(order);
                    std::array<double, filterTypes.size()> mse{};
                    std::array<std::vector<double>, filterTypes.size()> sols{};
                    // for all filter types
                    for (size_t j = 0; j < filterTypes.size(); j++) {
                        mFilter.setFilterType(filterTypes[j]);
                        sols[j].resize(initSol.size());
                        std::copy(initSol.begin(), initSol.end(), sols[j].begin());
                        mse[j] = improveSolution(sols[j], mAlgos2, startIdx, endIdx);
                        if (shouldExit.load()) { return; }
                        mse[j] = improveSolution(sols[j], mAlgos1, startIdx, endIdx);
                        if (shouldExit.load()) { return; }
                    }
                    // choose the filter type with the min mse
                    const auto idx = static_cast<size_t>(std::min_element(mse.begin(), mse.end()) - mse.begin());
                    mseByOrders.push_back(mse[idx]);
                    filterTypeByOrders.push_back(filterTypes[idx]);
                    solsByOrders.push_back(sols[idx]);
                }
                // choose the order with the min mse
                const auto idx = static_cast<size_t>(
                    std::min_element(mseByOrders.begin(), mseByOrders.end()) - mseByOrders.begin());
                const size_t actualIdx = mseByOrders[idx] <= mseByOrders[0] - highOrderEps
                                             ? idx
                                             : static_cast<size_t>(0);
                // store optimal mse and filter parameters
                mseS[i] = mseByOrders[actualIdx];
                filters[i].setFilterType(filterTypeByOrders[actualIdx]);
                filters[i].setOrder(orders[actualIdx]);
                filters[i].setFreq(std::exp(solsByOrders[actualIdx][0]));
                filters[i].setGain(solsByOrders[actualIdx][1] / gainScale);
                filters[i].setQ(std::exp(solsByOrders[actualIdx][2]));
                // update diff
                updateDiff(filters[i]);
                // if mse is already small enough, exit
                if (mseS[i] < eps) {
                    for (size_t j = i + 1; j < filters.size(); j++) {
                        mseS[j] = mseS[i];
                        filters[j].setFilterType(zlFilter::FilterType::peak);
                        filters[j].setOrder(2);
                        filters[j].setFreq(500.);
                        filters[j].setGain(0.);
                        filters[j].setQ(0.707);
                    }
                    return;
                }
                // if there is no chance to reach highOrderEps with higher orders, reduce order to 2
                if (mseS[i] < highOrderEps) {
                    orders = {2};
                }
            }
        }

        std::array<double, FilterNum> &getMSE() { return mseS; }

        std::array<zlFilter::Empty<double>, FilterNum> &getSol() { return filters; }

        void exitSignalSent() override {
            shouldExit.store(true);
        }

    private:
        std::array<zlFilter::Empty<double>, FilterNum> filters;
        std::array<double, FilterNum> mseS{};
        zlFilter::Ideal<double, maximumOrder> mFilter;
        std::vector<double> mDiffs;
        std::vector<double> mWs;
        std::atomic<bool> shouldExit{false};

        struct optFData {
            size_t startIdx;
            size_t endIdx;
            zlFilter::Ideal<double, maximumOrder> *filter;
            std::vector<double> *diffs;
            std::vector<double> *ws;
        };

        static double func(const std::vector<double> &x, std::vector<double> &grad, void *f_data) {
            auto *data = static_cast<optFData *>(f_data);
            const auto mse = calculateMSE(x[0], x[1], x[2],
                                          data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
            if (!grad.empty()) {
                const auto mse0l = calculateMSE(x[0] - eps, x[1], x[2],
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                const auto mse0r = calculateMSE(x[0] + eps, x[1], x[2],
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                grad[0] = (mse0r - mse0l) / (2 * eps);
                const auto mse1l = calculateMSE(x[0], x[1] - eps, x[2],
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                const auto mse1r = calculateMSE(x[0], x[1] + eps, x[2],
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                grad[1] = (mse1r - mse1l) / (2 * eps);
                const auto mse2l = calculateMSE(x[0], x[1], x[2] - eps,
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                const auto mse2r = calculateMSE(x[0], x[1], x[2] + eps,
                                                data->filter, data->diffs, data->ws, data->startIdx, data->endIdx);
                grad[2] = (mse2r - mse2l) / (2 * eps);
            }
            return mse;
        }

        static double calculateMSE(const double freqLog, const double gain, const double qLog,
                                   zlFilter::Ideal<double, maximumOrder> *filter,
                                   const std::vector<double> *diffs, const std::vector<double> *ws,
                                   const size_t startIdx, const size_t endIdx) {
            filter->setFreq(std::exp(freqLog));
            filter->setGain(gain / gainScale);
            filter->setQ(std::exp(qLog));
            filter->updateMagnitude(*ws);
            const auto &dB = filter->getDBs();
            double mse = 0.0;
            for (size_t i = startIdx; i < endIdx; ++i) {
                const auto t = dB[i] - diffs->at(i);
                mse += t * t;
            }
            return mse / static_cast<double>(dB.size());
        }

        double improveSolution(std::vector<double> &sol,
                               const std::vector<nlopt::algorithm> &algos,
                               const size_t startIdx, const size_t endIdx) {
            optFData optData{startIdx, endIdx, &mFilter, &mDiffs, &mWs};
            double bestMSE = 1e6;
            std::vector<double> bestSol = sol;
            const std::vector<double> mLowerBound{minFreqLog, minGainScale, minQLog};
            const std::vector<double> mUpperBound{maxFreqLog, maxGainScale, maxQLog};
            for (const auto &algo: algos) {
                if (shouldExit.load()) { return 0.f; }
                auto opt = nlopt::opt(algo, 3);
                opt.set_min_objective(func, &optData);
                opt.set_lower_bounds(mLowerBound);
                opt.set_upper_bounds(mUpperBound);
                opt.set_stopval(eps);
                opt.set_xtol_abs(1e-3);
                opt.set_population(80);
                opt.set_maxtime(1);
                try {
                    std::vector<double> currentSol = sol;
                    double currentMSE = 0.0;
                    const auto res = opt.optimize(currentSol, currentMSE);
                    if (res >= 0 && currentMSE < bestMSE) {
                        bestSol = currentSol;
                        bestMSE = currentMSE;
                    }
                } catch (...) {
                }
            }
            sol = bestSol;
            return bestMSE;
        }

        void updateDiff(const zlFilter::Empty<double> &eFilter) {
            mFilter.setFilterType(eFilter.getFilterType());
            mFilter.setOrder(eFilter.getOrder());
            mFilter.setFreq(eFilter.getFreq());
            mFilter.setGain(eFilter.getGain());
            mFilter.setQ(eFilter.getQ());
            mFilter.updateMagnitude(mWs);
            const auto &dB = mFilter.getDBs();
            for (size_t i = 0; i < dB.size(); ++i) {
                mDiffs[i] -= dB[i];
            }
        }
    };
} // zlEqMatch
