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
#include <optional>

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
        constexpr double kMaxGain = 29.999, kSeedGain = 3.0;
        // original MSE = mean squared dB error * ln(10)^2
        constexpr double kMseScale = std::numbers::ln10 * std::numbers::ln10;
        constexpr double kMinImprovement = 0.001 / kMseScale;
        constexpr double kSuggestedImprovement = 1.0 / kMseScale;
        constexpr float kResidualRmsDb = 0.20f, kResidualPeakDb = 0.75f;
        constexpr float kCancellationWeight = 0.02f;
        const float kCancellationScale = std::sqrt(kCancellationWeight);

        bool withinTolerance(const std::span<const float> residual) {
            return zldsp::vector::max_abs_of(residual.data(), residual.size()) <= kResidualPeakDb
                && zldsp::vector::sum_sqr(residual.data(), residual.size())
                    <= static_cast<double>(residual.size()) * (kResidualRmsDb * kResidualRmsDb);
        }

        struct Shape {
            Band band;
            double energy, inverse_energy;
        };

        struct ShapeGroup {
            std::vector<Shape> shapes;
            Curve responses;
        };

        struct Seeds {
            Band best, second_order;
        };

        struct Model {
            std::vector<Band> bands;
            Curve residual;
        };

        struct Evaluation {
            // the first block holds the dB residual
            Curve residual, band_responses, cancellation_slope;

            void resize(const size_t points, const size_t bands, const bool penalize) {
                residual.resize(penalize ? 2 * points : points);
                band_responses.resize(bands * points);
                cancellation_slope.resize(penalize ? points : 0);
            }
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
                  should_stop_(should_stop), ws_(freqs.size()) {
                filter_.prepare(sample_rate);
                const auto scale = static_cast<float>(2 * std::numbers::pi / sample_rate);
                zldsp::vector::multiply(ws_.data(), freqs.data(), scale, ws_.size());
            }

            FitResult run(const std::span<const float> target, const size_t max_bands) {
                FitResult result;
                prepareShapes();
                const auto num_points = static_cast<double>(target.size());
                double score = zldsp::vector::sum_sqr(target.data(), target.size()) / num_points;
                Model full{{}, Curve(target.begin(), target.end())};
                std::optional<Model> second_order;
                for (size_t count = 1; count <= max_bands && !should_stop_(); ++count) {
                    auto seeds = selectSeeds(full.residual, false);
                    // both searches have the same history until the full search picks a higher order.
                    if (second_order) {
                        seeds.second_order = selectSeeds(second_order->residual, true).best;
                    } else if (seeds.best.order != 2) {
                        second_order = full;
                    }
                    if (should_stop_()) { break; }
                    const Model* best = &full;
                    double next_score = extend(full, seeds.best, target) / num_points;
                    if (second_order && !should_stop_()) {
                        const auto other_score = extend(*second_order, seeds.second_order, target) / num_points;
                        if (other_score <= next_score || !std::isfinite(next_score)) {
                            next_score = other_score;
                            best = &*second_order;
                        }
                    }
                    const auto improvement = score - next_score;
                    if (should_stop_() || !(improvement > kMinImprovement)) { break; }

                    auto& selected = result.by_band_count.emplace_back();
                    selected.reserve(count);
                    for (const auto& band : best->bands) {
                        selected.push_back(band.parameters());
                    }
                    if (count > 1 && improvement < kSuggestedImprovement && result.suggested_num_bands == 0) {
                        result.suggested_num_bands = count;
                    }
                    score = next_score;
                    if (withinTolerance(best->residual)) { break; }
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
            Curve ws_;
            std::array<ShapeGroup, kTypes.size() * kOrders.size()> shape_groups_;
            Evaluation current_, candidate_;
            Curve jacobian_, band_slope_;
            std::vector<std::span<float>> columns_;
            std::vector<double> normal_, gradient_, system_, step_;
            std::vector<Band> trial_;

            void response(const Band& band, const std::span<float> out) {
                filter_.forceUpdate(band.parameters());
                filter_.updateMagnitudeSquare(ws_, out);
                zldsp::vector::sqr_mag_to_db(out.data(), out.size());
            }

            double evaluate(const std::span<const Band> bands, const std::span<const float> target, Evaluation& state) {
                const auto n = target.size();
                std::ranges::copy(target, state.residual.begin());
                auto penalty = std::span{state.residual}.subspan(n);
                // accumulate boost and cut in the two buffers used by the penalty and its derivative.
                std::ranges::fill(penalty, 0.f);
                std::ranges::fill(state.cancellation_slope, 0.f);
                for (size_t b = 0; b < bands.size(); ++b) {
                    auto db = std::span{state.band_responses}.subspan(b * n, n);
                    response(bands[b], db);
                    zldsp::vector::sub(state.residual.data(), db.data(), n);
                    if (!penalty.empty() && bands[b].type == FilterType::kPeak) {
                        for (size_t i = 0; i < n; ++i) {
                            state.cancellation_slope[i] += std::max(db[i], 0.f);
                            penalty[i] += std::max(-db[i], 0.f);
                        }
                    }
                }
                for (size_t i = 0; i < penalty.size(); ++i) {
                    const auto boost = state.cancellation_slope[i], cut = penalty[i];
                    penalty[i] = -kCancellationScale * std::min(boost, cut);
                    state.cancellation_slope[i] = boost <= cut ? kCancellationScale : -kCancellationScale;
                }
                return zldsp::vector::sum_sqr(state.residual.data(), state.residual.size());
            }

            // cache coarse responses to seed the band search
            void prepareShapes() {
                static constexpr std::array widths{0.3, 0.8, 2.5, 8.0};
                const auto octaves = (upper_[0] - lower_[0]) / std::numbers::ln2;
                const auto intervals = static_cast<size_t>(std::max(1., std::ceil(6. * octaves)));
                size_t group = 0;
                for (const auto type : kTypes) {
                    for (const auto order : kOrders) {
                        auto& bank = shape_groups_[group++];
                        const auto capacity = (intervals + 1) * widths.size();
                        bank.shapes.reserve(capacity);
                        bank.responses.resize(capacity * ws_.size());
                        for (size_t f = 0; f <= intervals; ++f) {
                            if (should_stop_()) { return; }
                            const auto position = static_cast<double>(f) / static_cast<double>(intervals);
                            const auto log_freq = std::lerp(lower_[0], upper_[0], position);
                            for (const auto q : widths) {
                                Band band{type, order, {log_freq, kSeedGain, std::log(q)}};
                                auto db = std::span{bank.responses}.subspan(bank.shapes.size() * ws_.size(), ws_.size());
                                response(band, db);
                                const auto energy = zldsp::vector::sum_sqr(db.data(), db.size());
                                if (energy > 1e-10f) {
                                    bank.shapes.push_back({band, energy, 1. / static_cast<double>(energy)});
                                }
                            }
                        }
                    }
                }
            }

            Seeds selectSeeds(const std::span<const float> target, const bool second_order_only) {
                constexpr auto max_scale = kMaxGain / kSeedGain;
                const Band flat{FilterType::kPeak, 2, {lower_[0], 0., 0.}};
                Seeds seeds{flat, flat};
                double best_error = zldsp::vector::sum_sqr(target.data(), target.size());
                double second_order_error = best_error;
                for (const auto& bank : shape_groups_) {
                    if (should_stop_()) { break; }
                    if (bank.shapes.empty() || (second_order_only && bank.shapes.front().band.order != 2)) { continue; }
                    Band seed = flat;
                    double best_reduction = 0.;
                    const auto* db = bank.responses.data();
                    for (const auto& shape : bank.shapes) {
                        // project the residual onto the filter response
                        const double dot = zldsp::vector::dot_product(db, target.data(), target.size());
                        db += target.size();
                        const auto scale = std::clamp(dot * shape.inverse_energy, -max_scale, max_scale);
                        const auto reduction = scale * (2. * dot - scale * shape.energy);
                        if (reduction > best_reduction) {
                            best_reduction = reduction;
                            seed = shape.band;
                            seed.x[1] = scale * kSeedGain;
                        }
                    }
                    if (best_reduction > 0.) {
                        const auto error = refine(std::span{&seed, 1}, target, 16);
                        if (error < best_error) {
                            best_error = error;
                            seeds.best = seed;
                        }
                        if (seed.order == 2 && error < second_order_error) {
                            second_order_error = error;
                            seeds.second_order = seed;
                        }
                    }
                }
                return seeds;
            }

            // band responses add in dB, so differentiate one band at a time
            void linearize(const std::span<const Band> bands) {
                constexpr Coordinates delta{0.0005, 0.005, 0.0005};
                const auto n = ws_.size();
                for (size_t b = 0; b < bands.size(); ++b) {
                    const auto base = std::span{current_.band_responses}.subspan(b * n, n);
                    const bool penalize = columns_[3 * b].size() > n;
                    if (penalize) {
                        // only peaks on the smaller side of min(boost, cut) contribute to its derivative.
                        for (size_t i = 0; i < n; ++i) {
                            const float slope = current_.cancellation_slope[i];
                            band_slope_[i] = base[i] * slope > 0.f ? slope : 0.f;
                        }
                    }
                    for (size_t p = 0; p < delta.size(); ++p) {
                        auto shifted = bands[b];
                        const auto h = shifted.x[p] + delta[p] <= upper_[p] ? delta[p] : -delta[p];
                        shifted.x[p] += h;
                        auto column = columns_[3 * b + p];
                        response(shifted, column.first(n));
                        const auto inv_h = static_cast<float>(1. / h);
                        for (size_t i = 0; i < n; ++i) {
                            column[i] = (column[i] - base[i]) * inv_h;
                        }
                        if (penalize) {
                            zldsp::vector::multiply(column.data() + n, column.data(), band_slope_.data(), n);
                        }
                    }
                }
                for (size_t i = 0; i < columns_.size(); ++i) {
                    const auto column = columns_[i];
                    gradient_[i] = zldsp::vector::dot_product(column.data(), current_.residual.data(), column.size());
                    for (size_t j = i; j < columns_.size(); ++j) {
                        const auto other = columns_[j];
                        normal_[i * columns_.size() + j] = zldsp::vector::dot_product(
                            column.data(), other.data(), std::min(column.size(), other.size()));
                    }
                }
            }

            // refine the fit with bounded Levenberg-Marquardt
            double refine(const std::span<Band> bands, const std::span<const float> target, const size_t iterations) {
                const auto n = target.size(), dimensions = 3 * bands.size();
                const auto peaks = static_cast<size_t>(std::ranges::count(bands, FilterType::kPeak, &Band::type));
                current_.resize(n, bands.size(), peaks > 1);
                auto error = evaluate(bands, target, current_);
                if (iterations == 0) { return error; }
                candidate_.resize(n, bands.size(), peaks > 1);
                // only peak columns can have nonzero cancellation rows.
                if (peaks > 1) { band_slope_.resize(n); }
                jacobian_.resize(n * (dimensions + (peaks > 1 ? 3 * peaks : 0)));
                columns_.resize(dimensions);
                auto remaining = std::span{jacobian_};
                for (size_t i = 0; i < dimensions; ++i) {
                    const auto length = peaks > 1 && bands[i / 3].type == FilterType::kPeak ? 2 * n : n;
                    columns_[i] = remaining.first(length);
                    remaining = remaining.subspan(length);
                }
                trial_.assign(bands.begin(), bands.end());
                normal_.resize(dimensions * dimensions);
                gradient_.resize(dimensions);
                double damping = 0.01;
                bool moved = true;
                for (size_t iteration = 0; iteration < iterations && !should_stop_(); ++iteration) {
                    // reuse the Jacobian after a rejected step
                    if (moved) {
                        linearize(bands);
                    }
                    system_ = normal_;
                    step_ = gradient_;
                    for (size_t i = 0; i < dimensions; ++i) {
                        auto& diag = system_[i * dimensions + i];
                        diag += damping * std::max(1.e-6, diag);
                    }
                    double next = std::numeric_limits<double>::infinity();
                    if (solve(system_, step_)) {
                        for (size_t b = 0; b < bands.size(); ++b) {
                            for (size_t p = 0; p < 3; ++p) {
                                trial_[b].x[p] = std::clamp(bands[b].x[p] + step_[3 * b + p], lower_[p], upper_[p]);
                            }
                        }
                        next = evaluate(trial_, target, candidate_);
                    }
                    moved = next < error;
                    if (moved) {
                        const auto improvement = error - next;
                        std::ranges::copy(trial_, bands.begin());
                        std::swap(current_, candidate_);
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

            double extend(Model& model, const Band& seed, const std::span<const float> target) {
                model.bands.push_back(seed);
                const auto score = refine(model.bands, target, model.bands.size() == 1 ? 0 : 22);
                std::copy_n(current_.residual.begin(), target.size(), model.residual.begin());
                return score;
            }
        };
    }

    FitResult fit(const double sample_rate, const std::span<const float> freqs, const std::span<const float> target_db,
                  const size_t max_bands, const std::function<bool()>& should_stop) {
        if (max_bands == 0 || freqs.empty() || freqs.size() != target_db.size()
            || !std::isfinite(sample_rate) || sample_rate * kMaxFreqRatio <= kMinFreq
            || !std::ranges::all_of(freqs, [](float f) { return std::isfinite(f) && f >= 0.f; })
            || !std::ranges::all_of(target_db, [](float db) { return std::isfinite(db); })
            || withinTolerance(target_db) || should_stop()) {
            return {};
        }
        return Fitter(sample_rate, freqs, should_stop).run(target_db, max_bands);
    }
}
