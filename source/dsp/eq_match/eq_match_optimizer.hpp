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

#include "../filter/ideal_filter/ideal.hpp"

namespace zldsp::eq_match {
    class EqMatchOptimizer final {
    public:
        EqMatchOptimizer(const double sample_rate,
                         const std::span<float> freqs,
                         const std::span<float> diffs) :
            lower_bound_({std::log(10.1), kMinGainScale, kMinQLog}),
            upper_bound_({std::log(sample_rate * (19.9 / 44.1)), kMaxGainScale, kMaxQLog}) {
            filter_.prepare(sample_rate);

            diffs_.resize(diffs.size());
            for (size_t i = 0; i < diffs.size(); i++) {
                diffs_[i] = static_cast<double>(diffs[i]) / 10.;
            }

            const auto w_scale = static_cast<float>(2 * std::numbers::pi / sample_rate);
            ws_.resize(freqs.size());
            for (size_t i = 0; i < freqs.size(); ++i) {
                ws_[i] = freqs[i] * w_scale;
            }

            res_.resize(freqs.size());
        }

        /**
         * fit current diff with filters
         * @param paras where to store filters' paras
         * @param num_band maximum number of bands
         * @param should_return check whether it should return early (like thread exit)
         * @param suggest_threshold suggest threshold
         * @return the suggest number of band that can make MSE less than suggest threshold
         */
        size_t fit(std::vector<zldsp::filter::FilterParameters>& paras,
                   const size_t num_band,
                   const std::function<bool()>& should_return,
                   const double suggest_threshold = 1.0) {
            paras.clear();
            double sum_sqr{0.};
            for (const auto& v: diffs_) {
                sum_sqr += v * v;
            }
            if (sum_sqr < kEps) {
                return 0;
            }
            std::vector filter_types{
                zldsp::filter::FilterType::kPeak,
                zldsp::filter::FilterType::kLowShelf,
                zldsp::filter::FilterType::kHighShelf
            };
            std::vector<size_t> filter_orders{2, 4, 6};
            size_t suggest_num_band = 0;
            double previous_mse = 1e6;
            for (size_t band = 0; band < num_band; ++band) {
                const auto [para, mse] = addOneFilter(filter_types, filter_orders);
                paras.emplace_back(para);
                const auto delta_mse = previous_mse - mse;
                previous_mse = mse;
                if (delta_mse < suggest_threshold && suggest_num_band == 0) {
                    suggest_num_band = band + 1;
                }
                if (delta_mse < kHighOrderEps) {
                    filter_orders = {2};
                }
                if (delta_mse < kEps) {
                    return suggest_num_band > 0 ? suggest_num_band : band;
                }
                if (should_return()) {
                    return suggest_num_band > 0 ? suggest_num_band : band;
                }
            }
            return suggest_num_band > 0 ? suggest_num_band : num_band;
        }

        /**
         * get the best filter for current diff and update the diff
         * @param filter_types all filter types
         * @param filter_orders all filter orders
         * @return the best filter para and MSE
         */
        std::tuple<zldsp::filter::FilterParameters, double> addOneFilter(
            const std::span<zldsp::filter::FilterType> filter_types,
            const std::span<size_t> filter_orders) {
            double best_mse = 1e6;
            zldsp::filter::FilterParameters best_para{};
            // find the best filter among all combinations of filter types and filter orders
            for (const auto& filter_type : filter_types) {
                const size_t sol_size{3};
                filter_.setFilterType(filter_type);
                for (const auto& filter_order : filter_orders) {
                    filter_.setOrder(filter_order);
                    std::vector<double> sol(kInitSol.begin(), kInitSol.begin() + sol_size);
                    fitFGQ(sol, kAlgos2);
                    const auto mse = fitFGQ(sol, kAlgos1);
                    if (mse < best_mse) {
                        best_mse = mse;
                        best_para.filter_type = filter_type;
                        best_para.order = filter_order;
                        best_para.freq = std::exp(sol[0]);
                        best_para.gain = sol[1] / kGainScale;
                        best_para.q = std::exp(sol[2]);
                    }
                }
            }
            // update diff with the best filter
            filter_.setFilterType(best_para.filter_type);
            filter_.setOrder(best_para.order);
            filter_.setFreq(best_para.freq);
            filter_.setGain(best_para.gain);
            filter_.setQ(best_para.q);
            filter_.updateCoeffs();
            filter_.updateMagnitudeSquare(ws_, res_);
            for (size_t i = 0; i < diffs_.size(); ++i) {
                diffs_[i] -= std::log10(res_[i]);
            }
            return {best_para, best_mse};
        }

    private:
        static constexpr double kEps = 1e-3, kHighOrderEps = 1.;

        static constexpr std::array kAlgos1{
            nlopt::algorithm::LD_MMA, nlopt::algorithm::LD_SLSQP, nlopt::algorithm::LD_VAR2
        };
        static constexpr std::array kAlgos2{
            nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::GN_CRS2_LM, nlopt::algorithm::LD_SLSQP
        };

        static constexpr double kMinGain = -29.999, kMaxGain = 29.999, kGainScale = .15;
        static constexpr double kMinGainScale = kMinGain * kGainScale;
        static constexpr double kMaxGainScale = kMaxGain * kGainScale;
        static constexpr double kMinQLog = -2.3025850929940455, kMaxQLog = 2.302585092994046;
        static constexpr std::array<double, 3> kInitSol{6.214608098422191, 0.0, -0.3465735902799726};

        const std::array<double, 3> lower_bound_;
        const std::array<double, 3> upper_bound_;

        zldsp::filter::Ideal<double, 6> filter_;
        std::vector<double> ws_;
        std::vector<double> diffs_;
        std::vector<double> res_;

        struct OptFData {
            size_t n;
            zldsp::filter::Ideal<double, 6>* filter;
            double* ws;
            double* diffs;
            double* res;
        };

        /**
         * calculate MSE error of the
         * @param x freq (& gain & q) value
         * @param data outside data
         * @return
         */
        template <size_t sol_size>
        static double calculateMSE(const std::span<double> x, const OptFData* data) {
            const auto filter = data->filter;
            if constexpr (sol_size >= 1) {
                filter->setFreq(std::exp(x[0]));
                if constexpr (sol_size >= 2) {
                    filter->setGain(x[1] / kGainScale);
                    if constexpr (sol_size >= 3) {
                        filter->setQ(std::exp(x[2]));
                    }
                }
            }
            filter->updateCoeffs();
            filter->updateMagnitudeSquare(std::span(data->ws, data->n),
                                          std::span(data->res, data->n));
            double sum_sqr{0.};
            for (size_t i = 0; i < data->n; ++i) {
                const auto err = data->diffs[i] - std::log10(data->res[i]);
                sum_sqr += err * err;
            }
            return 100. * sum_sqr / static_cast<double>(data->n);
        }

        /**
         * calculate error and gradient at the current solution
         * @param x freq (& gain & q) value
         * @param grad gradient
         * @param f_data outside data
         * @return
         */
        template <size_t sol_size>
        static double func(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
            const auto* data = static_cast<OptFData*>(f_data);
            std::array<double, sol_size> x_temp;
            for (size_t i = 0; i < sol_size; ++i) {
                x_temp[i] = x[i];
            }
            const auto mse = calculateMSE<sol_size>(x_temp, data);
            if (!grad.empty()) {
                for (size_t i = 0; i < sol_size; ++i) {
                    const auto xi = x[i];
                    x_temp[i] = xi - kEps;
                    const auto mse_l = calculateMSE<sol_size>(x_temp, data);
                    x_temp[i] = xi + kEps;
                    const auto mse_r = calculateMSE<sol_size>(x_temp, data);
                    grad[i] = (mse_r - mse_l) / (2.0 * kEps);
                }
            }
            return mse;
        }

        /**
         * fit freq (& gain & q) value of a given filter
         * filter type and filter order should have been assigned
         * @param sol the solution vector, contains at most three doubles
         * @param algos a list of algorithms
         * @return
         */
        double fitFGQ(std::vector<double>& sol,
                      const std::span<const nlopt::algorithm> algos) {
            OptFData data{ws_.size(), &filter_, ws_.data(), diffs_.data(), res_.data()};
            double best_mse = 1e6;
            std::vector<double> best_sol{sol.begin(), sol.end()};
            const std::vector<double> lower_bound{lower_bound_.begin(), lower_bound_.begin() + sol.size()};
            const std::vector<double> upper_bound{upper_bound_.begin(), upper_bound_.begin() + sol.size()};
            for (const auto& algo : algos) {
                auto opt = nlopt::opt(algo, 3);
                opt.set_min_objective(func<3>, &data);
                opt.set_lower_bounds(lower_bound);
                opt.set_upper_bounds(upper_bound);
                opt.set_stopval(kEps);
                opt.set_xtol_abs(kEps);
                opt.set_population(80);
                opt.set_maxtime(1);
                try {
                    std::vector<double> c_sol{sol.begin(), sol.end()};
                    double c_mse = 1e6;
                    const auto res = opt.optimize(c_sol, c_mse);
                    if (res >= 0 && c_mse < best_mse) {
                        best_mse = c_mse;
                        best_sol.assign(c_sol.begin(), c_sol.end());
                    }
                }
                catch (...) {
                }
            }
            sol.assign(best_sol.begin(), best_sol.end());
            return best_mse;
        }
    };
}
