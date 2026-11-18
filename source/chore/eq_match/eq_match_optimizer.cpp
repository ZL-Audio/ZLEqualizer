// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "eq_match_optimizer.hpp"

#include <limits>

#include "../../dsp/filter/ideal_filter/ideal.hpp"
#include "../../dsp/vector/vector.hpp"
#include <hwy/contrib/algo/transform-inl.h>

namespace zlchore::eq_match {
    namespace {
        using zldsp::filter::FilterParameters;
        using zldsp::filter::FilterType;
        using Curve = std::vector<float>;
        using Coordinates = std::array<double, 3>; // log(Hz), dB, log(Q)
        constexpr double kMinFreq = 10.1, kMaxFreqRatio = 19.9 / 44.1;

        struct Band {
            FilterType type;
            size_t order;
            Coordinates x;

            [[nodiscard]] FilterParameters parameters() const {
                return {type, order, std::exp(x[0]), x[1], std::exp(x[2])};
            }
        };

        constexpr std::array kTypes{FilterType::kPeak, FilterType::kLowShelf, FilterType::kHighShelf};
        constexpr std::array<size_t, 3> kOrders{2, 4, 6};
        constexpr double kMaxGain = 29.999;
        // original MSE = mean squared dB error * ln(10)^2
        constexpr double kMseScale = std::numbers::ln10 * std::numbers::ln10;
        constexpr double kMinImprovement = 0.001 / kMseScale;
        constexpr double kSuggestedImprovement = 1.0 / kMseScale;

        struct Shape {
            Band band;
            Curve response;
            double energy;
        };

        struct Model {
            std::vector<Band> bands;
            Curve residual;
        };

        // update contiguous rows during Cholesky factorization
        bool solve(std::vector<double>& a, std::vector<double>& rhs) {
            namespace hn = hwy::HWY_NAMESPACE;
            const hn::ScalableTag<double> d;
            const auto n = rhs.size();
            for (size_t i = 0; i < n; ++i) {
                auto* row = a.data() + i * n;
                if (!(row[i] > 0.)) { return false; }
                row[i] = std::sqrt(row[i]);
                rhs[i] /= row[i];
                for (size_t j = i + 1; j < n; ++j) {
                    row[j] /= row[i];
                }
                for (size_t j = i + 1; j < n; ++j) {
                    auto* next = a.data() + j * n;
                    const auto factor = hn::Set(d, row[j]);
                    hn::Transform1(d, next + j, n - j, row + j, [&](auto, auto value, auto pivot) HWY_ATTR {
                        return hn::NegMulAdd(factor, pivot, value);
                    });
                    rhs[j] -= row[j] * rhs[i];
                }
            }
            for (size_t i = n; i-- > 0;) {
                const auto* row = a.data() + i * n;
                for (size_t j = i + 1; j < n; ++j) {
                    rhs[i] -= row[j] * rhs[j];
                }
                rhs[i] /= row[i];
            }
            return true;
        }

        class Fitter {
        public:
            Fitter(const double sample_rate, const std::span<const float> freqs,
                   const std::function<bool()>& should_stop)
                : upper_{std::log(std::min(160000., sample_rate * kMaxFreqRatio)), kMaxGain, std::log(10.)},
                  should_stop_(should_stop), ws_(freqs.size()), response_(freqs.size()) {
                filter_.prepare(sample_rate);
                const auto scale = static_cast<float>(2 * std::numbers::pi / sample_rate);
                zldsp::vector::multiply(ws_.data(), freqs.data(), scale, ws_.size());
            }

            FitResult run(const std::span<const float> target, const size_t max_bands) {
                FitResult result;
                const auto num_points = static_cast<double>(target.size());
                double mse = zldsp::vector::sum_sqr(target.data(), target.size()) / num_points;
                if (mse < kMinImprovement || should_stop_()) {
                    return result;
                }
                prepareShapes();
                // keep a second-order alternative for overlapping peaks
                std::array<Model, 2> models{};
                for (auto& model : models) {
                    model.residual.assign(target.begin(), target.end());
                }
                for (size_t count = 1; count <= max_bands && !should_stop_(); ++count) {
                    double next_mse = std::numeric_limits<double>::infinity();
                    const Model* best = nullptr;
                    for (size_t i = 0; i < models.size(); ++i) {
                        auto& model = models[i];
                        model.bands.push_back(selectBand(model.residual, i == 0));
                        if (should_stop_()) { break; }
                        if (count > 1) {
                            refine(model.bands, target, 22);
                        }
                        const auto error = getResidual(model.bands, target, model.residual) / num_points;
                        if (error < next_mse) {
                            best = &model;
                            next_mse = error;
                        }
                    }
                    const auto improvement = mse - next_mse;
                    if (should_stop_() || best == nullptr || improvement <= kMinImprovement) { break; }

                    auto& selected = result.by_band_count.emplace_back();
                    selected.reserve(count);
                    for (const auto& band : best->bands) {
                        selected.push_back(band.parameters());
                    }
                    if (count > 1 && improvement < kSuggestedImprovement && result.suggested_num_bands == 0) {
                        result.suggested_num_bands = count;
                    }
                    mse = next_mse;
                    if (mse < kMinImprovement) { break; }
                }
                if (result.suggested_num_bands == 0) {
                    result.suggested_num_bands = result.by_band_count.size() - 1;
                }
                return result;
            }

        private:
            const Coordinates lower_{std::log(kMinFreq), -kMaxGain, std::log(.1)};
            const Coordinates upper_;
            const std::function<bool()>& should_stop_;
            zldsp::filter::Ideal<float, 6> filter_;
            Curve ws_, response_;
            std::array<std::vector<Shape>, kTypes.size() * kOrders.size()> shapes_;

            void response(const Band& band, const std::span<float> out) {
                filter_.forceUpdate(band.parameters());
                filter_.updateMagnitudeSquare(ws_, out);
                zldsp::vector::sqr_mag_to_db(out.data(), out.size());
            }

            double getResidual(const std::span<const Band> bands, const std::span<const float> target, Curve& residual) {
                std::ranges::copy(target, residual.begin());
                for (const auto& band : bands) {
                    response(band, response_);
                    zldsp::vector::sub(residual.data(), response_.data(), residual.size());
                }
                return zldsp::vector::sum_sqr(residual.data(), residual.size());
            }

            // cache coarse responses to seed the band search
            void prepareShapes() {
                static constexpr double seed_gain = 3.0;
                static constexpr std::array widths{0.3, 0.8, 2.5, 8.0};
                const auto octaves = (upper_[0] - lower_[0]) / std::numbers::ln2;
                const auto intervals = static_cast<size_t>(std::max(1., std::ceil(6. * octaves)));
                size_t group = 0;
                for (const auto type : kTypes) {
                    for (const auto order : kOrders) {
                        auto& shapes = shapes_[group++];
                        shapes.reserve((intervals + 1) * widths.size());
                        for (size_t f = 0; f <= intervals; ++f) {
                            if (should_stop_()) { return; }
                            const auto position = static_cast<double>(f) / static_cast<double>(intervals);
                            const auto log_freq = std::lerp(lower_[0], upper_[0], position);
                            for (const auto q : widths) {
                                Band band{type, order, {log_freq, seed_gain, std::log(q)}};
                                response(band, response_);
                                const auto energy = zldsp::vector::sum_sqr(response_.data(), response_.size());
                                if (energy > 1e-10f) {
                                    shapes.push_back({band, response_, energy});
                                }
                            }
                        }
                    }
                }
            }

            Band selectBand(const std::span<const float> target, const bool second_order_only) {
                Band best{FilterType::kPeak, 2, {lower_[0], 0., 0.}};
                double best_error = zldsp::vector::sum_sqr(target.data(), target.size());
                for (const auto& shapes : shapes_) {
                    if (should_stop_()) { break; }
                    if (shapes.empty() || (second_order_only && shapes.front().band.order != 2)) { continue; }
                    Band seed = best;
                    double best_reduction = 0.;
                    for (const auto& shape : shapes) {
                        // project the residual onto the filter response
                        const double dot = zldsp::vector::dot_product(shape.response.data(), target.data(), target.size());
                        const auto gain = std::clamp(shape.band.x[1] * dot / shape.energy, -kMaxGain, kMaxGain);
                        const auto scale = gain / shape.band.x[1];
                        const auto reduction = scale * (2. * dot - scale * shape.energy);
                        if (reduction > best_reduction) {
                            best_reduction = reduction;
                            seed = shape.band;
                            seed.x[1] = gain;
                        }
                    }
                    if (best_reduction > 0.) {
                        const auto error = refine(std::span{&seed, 1}, target, 16);
                        if (error < best_error) {
                            best_error = error;
                            best = seed;
                        }
                    }
                }
                return best;
            }

            // band responses add in dB, so differentiate one band at a time
            void linearize(const std::span<const Band> bands, const Curve& residual, Curve& jacobian,
                           std::vector<double>& normal, std::vector<double>& gradient) {
                constexpr Coordinates delta{0.0005, 0.005, 0.0005};
                const auto n = residual.size();
                for (size_t b = 0; b < bands.size(); ++b) {
                    response(bands[b], response_);
                    for (size_t p = 0; p < delta.size(); ++p) {
                        auto shifted = bands[b];
                        const auto h = shifted.x[p] + delta[p] <= upper_[p] ? delta[p] : -delta[p];
                        shifted.x[p] += h;
                        auto column = std::span{jacobian}.subspan((3 * b + p) * n, n);
                        response(shifted, column);
                        zldsp::vector::sub(column.data(), response_.data(), n);
                        zldsp::vector::multiply(column.data(), static_cast<float>(1. / h), n);
                    }
                }
                for (size_t i = 0; i < gradient.size(); ++i) {
                    const auto* column = jacobian.data() + i * n;
                    gradient[i] = zldsp::vector::dot_product(column, residual.data(), n);
                    for (size_t j = i; j < gradient.size(); ++j) {
                        normal[i * gradient.size() + j] = zldsp::vector::dot_product(column, jacobian.data() + j * n, n);
                    }
                }
            }

            // refine the fit with bounded Levenberg-Marquardt
            double refine(const std::span<Band> bands, const std::span<const float> target, const size_t iterations) {
                const auto n = target.size(), dimensions = 3 * bands.size();
                Curve residual(n), trial_residual(n), jacobian(n * dimensions);
                std::vector<Band> trial(bands.begin(), bands.end());
                std::vector<double> normal(dimensions * dimensions), gradient(dimensions), system, step;
                auto error = getResidual(bands, target, residual);
                double damping = 0.01;
                bool moved = true;
                for (size_t iteration = 0; iteration < iterations && !should_stop_(); ++iteration) {
                    // reuse the Jacobian after a rejected step
                    if (moved) {
                        linearize(bands, residual, jacobian, normal, gradient);
                    }
                    system = normal;
                    step = gradient;
                    for (size_t i = 0; i < dimensions; ++i) {
                        auto& diag = system[i * dimensions + i];
                        diag += damping * std::max(1.e-6, diag);
                    }
                    double next = std::numeric_limits<double>::infinity();
                    if (solve(system, step)) {
                        for (size_t b = 0; b < bands.size(); ++b) {
                            for (size_t p = 0; p < 3; ++p) {
                                trial[b].x[p] = std::clamp(bands[b].x[p] + step[3 * b + p], lower_[p], upper_[p]);
                            }
                        }
                        next = getResidual(trial, target, trial_residual);
                    }
                    moved = next < error;
                    if (moved) {
                        const auto improvement = error - next;
                        std::ranges::copy(trial, bands.begin());
                        std::swap(residual, trial_residual);
                        error = next;
                        damping = std::max(1.e-5, damping * .3);
                        if (improvement < 1.e-7 * std::max(1., error)) { break; }
                    } else {
                        damping *= 10.;
                        if (damping > 1.e5) { break; }
                    }
                }
                return error;
            }
        };
    }

    FitResult fit(const double sample_rate, const std::span<const float> freqs, const std::span<const float> target_db,
                  const size_t max_bands, const std::function<bool()>& should_stop) {
        if (max_bands == 0 || freqs.empty() || freqs.size() != target_db.size()
            || !std::isfinite(sample_rate) || sample_rate * kMaxFreqRatio <= kMinFreq
            || !std::ranges::all_of(freqs, [](float f) { return std::isfinite(f) && f >= 0.f; })
            || !std::ranges::all_of(target_db, [](float db) { return std::isfinite(db); })) {
            return {};
        }
        return Fitter(sample_rate, freqs, should_stop).run(target_db, max_bands);
    }
}
