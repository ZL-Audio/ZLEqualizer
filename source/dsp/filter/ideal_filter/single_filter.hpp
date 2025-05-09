// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "ideal_base.hpp"
#include "coeff/ideal_coeff.hpp"
#include "../filter_design/filter_design.hpp"

namespace zldsp::filter {
    /**
     * an ideal prototype filter which holds coeffs for calculating responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class Ideal {
    public:
        explicit Ideal() = default;

        void prepare(const double sampleRate) {
            fs_.store(sampleRate);
            to_update_para_.store(true);
        }

        void setFreq(const FloatType x) {
            freq_.store(x);
            to_update_para_.store(true);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq_.load()); }

        void setGain(const FloatType x) {
            if (std::abs(static_cast<double>(x) - gain_.load()) > 1e-6) {
                gain_.store(x);
                to_update_para_.store(true);
            }
        }

        FloatType getGain() const { return static_cast<FloatType>(gain_.load()); }

        void setQ(const FloatType x) {
            if (std::abs(static_cast<double>(x) - q_.load()) > 1e-6) {
                q_.store(x);
                to_update_para_.store(true);
            }
        }

        FloatType getQ() const { return static_cast<FloatType>(q_.load()); }

        void setFilterType(const FilterType x) {
            filter_type_.store(x);
            to_update_para_.store(true);
        }

        FilterType getFilterType() const { return filter_type_.load(); }

        void setOrder(const size_t x) {
            order_.store(x);
            to_update_para_.store(true);
        }

        size_t getOrder() const { return order_.load(); }

        void prepareResponseSize(const size_t x) {
            response_.resize(x);
            std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
        }

        void prepareDBSize(const size_t x) {
            dbs_.resize(x);
            gains_.resize(x);
        }

        bool getMagOutdated() const { return to_update_para_.load(); }

        bool updateResponse(const std::vector<std::complex<FloatType> > &wis) {
            if (to_update_para_.exchange(false)) {
                updateParas();
                std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::updateResponse(coeffs_[i], wis, response_);
                }
                return true;
            }
            return false;
        }

        bool updateMixPhaseResponse(const std::vector<std::complex<FloatType> > &wis,
                                    const size_t startMix, const size_t endMix, const std::vector<FloatType> &mix) {
            if (to_update_para_.exchange(false)) {
                updateParas();
                std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::updateMixResponse(coeffs_[i], wis, response_,
                                                            startMix, endMix, mix);
                }
                return true;
            }
            return false;
        }

        bool updateZeroPhaseResponse(const std::vector<std::complex<FloatType> > &wis) {
            if (to_update_para_.exchange(false)) {
                updateParas();
                std::fill(response_.begin(), response_.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::updateResponse(coeffs_[i], wis, response_);
                }
                for (size_t i = 0; i < response_.size(); ++i) {
                    response_[i] = std::complex<FloatType>(std::abs(response_[i]), FloatType(0));
                }
                return true;
            }
            return false;
        }

        bool updateMagnitude(const std::vector<FloatType> &ws) {
            if (to_update_para_.exchange(false)) {
                updateParas();
                std::fill(gains_.begin(), gains_.end(), FloatType(1));
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::updateMagnitude(coeffs_[i], ws, gains_);
                }
                std::transform(gains_.begin(), gains_.end(), dbs_.begin(),
                               [](auto &g) {
                                   return g > FloatType(0) ? std::log10(g) * FloatType(20) : FloatType(-480);
                               });
                return true;
            }
            return false;
        }

        void addDBs(std::vector<FloatType> &x) {
            std::transform(x.begin(), x.end(), dbs_.begin(), x.begin(),
                           [](auto &c1, auto &c2) { return c1 + c2; });
        }

        std::vector<FloatType> &getDBs() {
            return dbs_;
        }

        FloatType getDB(FloatType w) {
            double g0 = 1.0;
            for (size_t i = 0; i < current_filter_num_; ++i) {
                g0 *= IdealBase<FloatType>::getMagnitude(coeffs_[i], w);
            }
            return g0 > FloatType(0) ? std::log10(g0) * FloatType(20) : FloatType(-480);
        }

        std::vector<std::complex<FloatType> > &getResponse() { return response_; }

        void setToUpdate() { to_update_para_.store(true); }

    private:
        std::array<std::array<double, 6>, FilterSize> coeffs_{};
        std::atomic<bool> to_update_para_{true};
        std::atomic<size_t> order_{2};
        size_t current_filter_num_{1};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<double> fs_{48000.0};
        std::atomic<FilterType> filter_type_ = FilterType::kPeak;
        std::vector<FloatType> dbs_{}, gains_{};
        std::vector<std::complex<FloatType> > response_{};

        void updateParas() {
            current_filter_num_ = updateIIRCoeffs(filter_type_.load(), order_.load(),
                                               freq_.load(), fs_.load(),
                                               gain_.load(), q_.load(), coeffs_);
        }

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, FilterSize> &coeffs) {
            return FilterDesign::updateCoeffs<FilterSize,
                IdealCoeff::get1LowShelf, IdealCoeff::get1HighShelf, IdealCoeff::get1TiltShelf,
                IdealCoeff::get1LowPass, IdealCoeff::get1HighPass,
                IdealCoeff::get2Peak,
                IdealCoeff::get2LowShelf, IdealCoeff::get2HighShelf, IdealCoeff::get2TiltShelf,
                IdealCoeff::get2LowPass, IdealCoeff::get2HighPass,
                IdealCoeff::get2BandPass, IdealCoeff::get2Notch>(
                filterType, n, f, fs, g0, q0, coeffs);
        }
    };
}
