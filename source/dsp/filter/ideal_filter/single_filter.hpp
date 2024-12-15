// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_IDEAL_SINGLE_FILTER_HPP
#define ZLFILTER_IDEAL_SINGLE_FILTER_HPP

#include "ideal_base.hpp"
#include "coeff/ideal_coeff.hpp"
#include "../filter_design/filter_design.hpp"

namespace zlFilter {
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
            fs.store(sampleRate);
            toUpdatePara.store(true);
        }

        void setFreq(const FloatType x) {
            freq.store(x);
            toUpdatePara.store(true);
        }

        FloatType getFreq() const { return static_cast<FloatType>(freq.load()); }

        void setGain(const FloatType x) {
            if (std::abs(static_cast<double>(x) - gain.load()) > 1e-6) {
                gain.store(x);
                toUpdatePara.store(true);
            }
        }

        FloatType getGain() const { return static_cast<FloatType>(gain.load()); }

        void setQ(const FloatType x) {
            if (std::abs(static_cast<double>(x) - q.load()) > 1e-6) {
                q.store(x);
                toUpdatePara.store(true);
            }
        }

        FloatType getQ() const { return static_cast<FloatType>(q.load()); }

        void setFilterType(const FilterType x) {
            filterType.store(x);
            toUpdatePara.store(true);
        }

        FilterType getFilterType() const { return filterType.load(); }

        void setOrder(const size_t x) {
            order.store(x);
            toUpdatePara.store(true);
        }

        size_t getOrder() const { return order.load(); }

        void prepareResponseSize(const size_t x) {
            response.resize(x);
            std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
        }

        void prepareDBSize(const size_t x) {
            dBs.resize(x);
            gains.resize(x);
        }

        bool getMagOutdated() const { return toUpdatePara.load(); }

        bool updateResponse(const std::vector<std::complex<FloatType> > &wis) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
                std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    IdealBase<FloatType>::updateResponse(coeffs[i], wis, response);
                }
                return true;
            }
            return false;
        }

        bool updateMixPhaseResponse(const std::vector<std::complex<FloatType> > &wis,
                                    const size_t startMix, const size_t endMix, const std::vector<FloatType> &mix) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
                std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    IdealBase<FloatType>::updateMixResponse(coeffs[i], wis, response,
                                                            startMix, endMix, mix);
                }
                return true;
            }
            return false;
        }

        bool updateZeroPhaseResponse(const std::vector<std::complex<FloatType> > &wis) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
                std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    IdealBase<FloatType>::updateResponse(coeffs[i], wis, response);
                }
                for (size_t i = 0; i < response.size(); ++i) {
                    response[i] = std::complex<FloatType>(std::abs(response[i]), FloatType(0));
                }
                return true;
            }
            return false;
        }

        bool updateMagnitude(const std::vector<FloatType> &ws) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
                std::fill(gains.begin(), gains.end(), FloatType(1));
                for (size_t i = 0; i < currentFilterNum; ++i) {
                    IdealBase<FloatType>::updateMagnitude(coeffs[i], ws, gains);
                }
                std::transform(gains.begin(), gains.end(), dBs.begin(),
                               [](auto &g) {
                                   return g > FloatType(0) ? std::log10(g) * FloatType(20) : FloatType(-480);
                               });
                return true;
            }
            return false;
        }

        void addDBs(std::vector<FloatType> &x) {
            std::transform(x.begin(), x.end(), dBs.begin(), x.begin(),
                           [](auto &c1, auto &c2) { return c1 + c2; });
        }

        std::vector<FloatType> &getDBs() {
            return dBs;
        }

        FloatType getDB(FloatType w) {
            double g0 = 1.0;
            for (size_t i = 0; i < currentFilterNum; ++i) {
                g0 *= IdealBase<FloatType>::getMagnitude(coeffs[i], w);
            }
            return g0 > FloatType(0) ? std::log10(g0) * FloatType(20) : FloatType(-480);
        }

        std::vector<std::complex<FloatType> > &getResponse() { return response; }

        void setToUpdate() { toUpdatePara.store(true); }

    private:
        std::array<std::array<double, 6>, FilterSize> coeffs{};
        std::atomic<bool> toUpdatePara{true};
        std::atomic<size_t> order{2};
        size_t currentFilterNum{1};
        std::atomic<double> freq{1000.0}, gain{0.0}, q{0.707};
        std::atomic<double> fs{48000.0};
        std::atomic<FilterType> filterType = FilterType::peak;
        std::vector<FloatType> dBs{}, gains{};
        std::vector<std::complex<FloatType> > response{};

        void updateParas() {
            currentFilterNum = updateIIRCoeffs(filterType.load(), order.load(),
                                               freq.load(), fs.load(),
                                               gain.load(), q.load(), coeffs);
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

#endif //ZLFILTER_IDEAL_SINGLE_FILTER_HPP
