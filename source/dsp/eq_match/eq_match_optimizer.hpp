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

namespace zldsp::eq_match {
    template<size_t FilterNum>
    class EqMatchOptimizer final : public juce::Thread::Listener {
    public:
        static constexpr double kEps = 1e-3, kHighOrderEps = .1;
        static constexpr double kDiffMinFreqLog = 2.302585092994046, kDiffMaxFreqLog = 9.998797732340453;
        static constexpr double kMinFreqLog = 2.3026, kMaxFreqLog = 9.9034;
        static constexpr double kMinGain = -29.999, kMaxGain = 29.999, kGainScale = .15;
        static constexpr double kMinGainScale = kMinGain * kGainScale, kMaxGainScale = kMaxGain * kGainScale;
        static constexpr double kMinQLog = -2.3025850929940455, kMaxQLog = 2.302585092994046;
        static constexpr std::array<double, 3> kLowerBound{kMinFreqLog, kMinGain * kGainScale, kMinQLog};
        static constexpr std::array<double, 3> kUpperBound{kMaxFreqLog, kMaxGain * kGainScale, kMaxQLog};
        static constexpr std::array kAlgos1{
            nlopt::algorithm::LD_MMA, nlopt::algorithm::LD_SLSQP, nlopt::algorithm::LD_VAR2
        };
        static constexpr std::array kAlgos2{
            nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::LD_SLSQP
        };
        static constexpr std::array<zldsp::filter::FilterType, 3> kFilterTypes{
            zldsp::filter::FilterType::kLowShelf, zldsp::filter::FilterType::kPeak,
            zldsp::filter::FilterType::kHighShelf
        };
        static constexpr size_t kMaximumOrder = 6;
        static constexpr std::array<double, 3> kInitSol{6.214608098422191, 0.0, -0.3465735902799726};

        EqMatchOptimizer() {
            filter_.prepare(48000.0);
        }

        std::vector<double> &getDiffs() { return diffs_; }

        void setDiffs(const double *diffs, const size_t diffs_size) {
            filter_.prepare(48000.0);
            filter_.prepareDBSize(diffs_size);
            const auto delta_log = (kDiffMaxFreqLog - kDiffMinFreqLog) / (static_cast<double>(diffs_size) - 1.0);
            auto current_log = kDiffMinFreqLog;
            ws_.resize(diffs_size);
            for (size_t i = 0; i < diffs_size; i++) {
                ws_[i] = std::exp(current_log) / 48000.0 * 2.0 * 3.141592653589793;
                current_log += delta_log;
            }
            diffs_.resize(diffs_size);
            for (size_t i = 0; i < diffs_size; i++) {
                diffs_[i] = diffs[i];
            }
        }

        void runDeterministic(size_t startIdx = 0, size_t endIdx = 10000) {
            to_exit_.store(false);
            endIdx = std::min(endIdx, diffs_.size() - 1);
            startIdx = std::min(startIdx, endIdx);
            std::vector<nlopt::algorithm> m_algos;
            m_algos.resize(kAlgos1.size());
            std::copy(kAlgos1.begin(), kAlgos1.end(), m_algos.begin());
            for (size_t i = 0; i < FilterNum; i++) {
                std::array<double, kFilterTypes.size()> mse{};
                std::array<std::vector<double>, kFilterTypes.size()> sols{};
                for (size_t j = 0; j < kFilterTypes.size(); j++) {
                    filter_.setFilterType(kFilterTypes[j]);
                    sols[j].resize(kInitSol.size());
                    std::copy(kInitSol.begin(), kInitSol.end(), sols[j].begin());
                    mse[j] = improveSolution(sols[j], m_algos, startIdx, endIdx);
                }
                const auto idx = static_cast<size_t>(std::min_element(mse.begin(), mse.end()) - mse.begin());
                mse_[idx] = mse[idx];
                filters_[i].setFilterType(kFilterTypes[idx]);
                filters_[i].setFreq(std::exp(sols[idx][0]));
                filters_[i].setGain(sols[idx][1] / kGainScale);
                filters_[i].setQ(std::exp(sols[idx][2]));
                updateDiff(filters_[i]);
                // if mse is already small enough, exit
                if (mse_[i] < kEps) {
                    for (size_t j = i + 1; j < filters_.size(); j++) {
                        mse_[j] = mse_[i];
                        filters_[j].setFilterType(zldsp::filter::FilterType::kPeak);
                        filters_[j].setFreq(500.);
                        filters_[j].setGain(0.);
                        filters_[j].setQ(0.707);
                    }
                    return;
                }
            }
        }

        void runStochastic(const size_t startIdx = 0, const size_t endIdx = 10000) {
            runStochasticPlus({2}, startIdx, endIdx);
        }

        void runStochasticPlus(std::vector<size_t> orders, size_t start_idx = 0, size_t end_idx = 10000) {
            to_exit_.store(false);
            end_idx = std::min(end_idx, diffs_.size());
            start_idx = std::min(start_idx, end_idx);
            std::vector<nlopt::algorithm> m_algos1, m_algos2;
            m_algos1.resize(kAlgos1.size());
            std::copy(kAlgos1.begin(), kAlgos1.end(), m_algos1.begin());
            m_algos2.resize(kAlgos2.size());
            std::copy(kAlgos2.begin(), kAlgos2.end(), m_algos2.begin());
            // fit filter one by one
            for (size_t i = 0; i < FilterNum; i++) {
                std::vector<double> mse_by_orders{};
                std::vector<zldsp::filter::FilterType> filter_type_by_orders{};
                std::vector<std::vector<double> > sols_by_orders{};
                // for all orders
                for (const size_t order: orders) {
                    filter_.setOrder(order);
                    std::array<double, kFilterTypes.size()> mse{};
                    std::array<std::vector<double>, kFilterTypes.size()> sols{};
                    // for all filter types
                    for (size_t j = 0; j < kFilterTypes.size(); j++) {
                        filter_.setFilterType(kFilterTypes[j]);
                        sols[j].resize(kInitSol.size());
                        std::copy(kInitSol.begin(), kInitSol.end(), sols[j].begin());
                        mse[j] = improveSolution(sols[j], m_algos2, start_idx, end_idx);
                        if (to_exit_.load()) { return; }
                        mse[j] = improveSolution(sols[j], m_algos1, start_idx, end_idx);
                        if (to_exit_.load()) { return; }
                    }
                    // choose the filter type with the min mse
                    const auto idx = static_cast<size_t>(std::min_element(mse.begin(), mse.end()) - mse.begin());
                    mse_by_orders.push_back(mse[idx]);
                    filter_type_by_orders.push_back(kFilterTypes[idx]);
                    sols_by_orders.push_back(sols[idx]);
                }
                // choose the order with the min mse
                const auto idx = static_cast<size_t>(
                    std::min_element(mse_by_orders.begin(), mse_by_orders.end()) - mse_by_orders.begin());
                const size_t actual_idx = mse_by_orders[idx] <= mse_by_orders[0] - kHighOrderEps
                                              ? idx
                                              : static_cast<size_t>(0);
                // store optimal mse and filter parameters
                mse_[i] = mse_by_orders[actual_idx];
                filters_[i].setFilterType(filter_type_by_orders[actual_idx]);
                filters_[i].setOrder(orders[actual_idx]);
                filters_[i].setFreq(std::exp(sols_by_orders[actual_idx][0]));
                filters_[i].setGain(sols_by_orders[actual_idx][1] / kGainScale);
                filters_[i].setQ(std::exp(sols_by_orders[actual_idx][2]));
                // update diff
                updateDiff(filters_[i]);
                // if mse is already small enough, exit
                if (mse_[i] < kEps) {
                    for (size_t j = i + 1; j < filters_.size(); j++) {
                        mse_[j] = mse_[i];
                        filters_[j].setFilterType(zldsp::filter::FilterType::kPeak);
                        filters_[j].setOrder(2);
                        filters_[j].setFreq(500.);
                        filters_[j].setGain(0.);
                        filters_[j].setQ(0.707);
                    }
                    return;
                }
                // if there is no chance to reach highOrderEps with higher orders, reduce order to 2
                if (mse_[i] < kHighOrderEps) {
                    orders = {2};
                }
            }
        }

        std::array<double, FilterNum> &getMSE() { return mse_; }

        std::array<zldsp::filter::Empty<double>, FilterNum> &getSol() { return filters_; }

        void exitSignalSent() override {
            to_exit_.store(true);
        }

    private:
        std::array<zldsp::filter::Empty<double>, FilterNum> filters_;
        std::array<double, FilterNum> mse_{};
        zldsp::filter::Ideal<double, kMaximumOrder> filter_;
        std::vector<double> diffs_;
        std::vector<double> ws_;
        std::atomic<bool> to_exit_{false};

        struct OptFData {
            size_t start_idx;
            size_t end_idx;
            zldsp::filter::Ideal<double, kMaximumOrder> *filter;
            std::vector<double> *diffs;
            std::vector<double> *ws;
        };

        static double func(const std::vector<double> &x, std::vector<double> &grad, void *f_data) {
            auto *data = static_cast<OptFData *>(f_data);
            const auto mse = calculateMSE(x[0], x[1], x[2],
                                          data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
            if (!grad.empty()) {
                const auto mse0l = calculateMSE(x[0] - kEps, x[1], x[2],
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                const auto mse0r = calculateMSE(x[0] + kEps, x[1], x[2],
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                grad[0] = (mse0r - mse0l) / (2 * kEps);
                const auto mse1l = calculateMSE(x[0], x[1] - kEps, x[2],
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                const auto mse1r = calculateMSE(x[0], x[1] + kEps, x[2],
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                grad[1] = (mse1r - mse1l) / (2 * kEps);
                const auto mse2l = calculateMSE(x[0], x[1], x[2] - kEps,
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                const auto mse2r = calculateMSE(x[0], x[1], x[2] + kEps,
                                                data->filter, data->diffs, data->ws, data->start_idx, data->end_idx);
                grad[2] = (mse2r - mse2l) / (2 * kEps);
            }
            return mse;
        }

        static double calculateMSE(const double freqLog, const double gain, const double qLog,
                                   zldsp::filter::Ideal<double, kMaximumOrder> *filter,
                                   const std::vector<double> *diffs, const std::vector<double> *ws,
                                   const size_t startIdx, const size_t endIdx) {
            filter->setFreq(std::exp(freqLog));
            filter->setGain(gain / kGainScale);
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
            OptFData optData{startIdx, endIdx, &filter_, &diffs_, &ws_};
            double bestMSE = 1e6;
            std::vector<double> bestSol = sol;
            const std::vector<double> mLowerBound{kMinFreqLog, kMinGainScale, kMinQLog};
            const std::vector<double> mUpperBound{kMaxFreqLog, kMaxGainScale, kMaxQLog};
            for (const auto &algo: algos) {
                if (to_exit_.load()) { return 0.f; }
                auto opt = nlopt::opt(algo, 3);
                opt.set_min_objective(func, &optData);
                opt.set_lower_bounds(mLowerBound);
                opt.set_upper_bounds(mUpperBound);
                opt.set_stopval(kEps);
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

        void updateDiff(const zldsp::filter::Empty<double> &eFilter) {
            filter_.setFilterType(eFilter.getFilterType());
            filter_.setOrder(eFilter.getOrder());
            filter_.setFreq(eFilter.getFreq());
            filter_.setGain(eFilter.getGain());
            filter_.setQ(eFilter.getQ());
            filter_.updateMagnitude(ws_);
            const auto &dB = filter_.getDBs();
            for (size_t i = 0; i < dB.size(); ++i) {
                diffs_[i] -= dB[i];
            }
        }
    };
} // zldsp::eq_match
