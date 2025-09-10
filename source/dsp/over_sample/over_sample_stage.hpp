// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <algorithm>

#include "../vector/kfr_import.hpp"

namespace zldsp::oversample {
    template<typename FloatType>
    class OverSampleStage {
    public:
        explicit OverSampleStage(std::span<const FloatType> up_coeff,
                                 std::span<const FloatType> down_coeff) {
            up_coeff_.resize(up_coeff.size() / 2);
            for (size_t i = 1; i < up_coeff.size(); i += 2) {
                up_coeff_[i >> 1] = up_coeff[i] * FloatType(2);
            }
            up_coeff_center_ = up_coeff[up_coeff.size() / 2] * FloatType(2);
            up_coeff_center_pos_ = up_coeff_.size() / 2;

            down_coeff_.resize(down_coeff.size() / 2);
            for (size_t i = 1; i < down_coeff.size(); i += 2) {
                down_coeff_[i >> 1] = down_coeff[i];
            }
            down_coeff_center_ = down_coeff[down_coeff.size() / 2];

            latency_ = (up_coeff.size() + down_coeff.size() - 2) / 4;
        }

        void prepare(const size_t num_channels, const size_t max_num_samples) {
            const size_t up_req_size = up_coeff_.size();
            const size_t down_req_size = down_coeff_.size();

            const size_t up_delay_size = up_req_size + max_num_samples + 1;
            up_delay_lines_.resize(num_channels);
            for (auto &d: up_delay_lines_) {
                d.resize(up_delay_size);
            }

            const size_t down_delay_size = down_req_size + max_num_samples + 1;
            down_delay_lines_.resize(num_channels);
            for (auto &d: down_delay_lines_) {
                d.resize(down_delay_size);
            }

            down_center_delay_lines_.resize(num_channels);
            for (auto &d: down_center_delay_lines_) {
                d.resize(down_coeff_.size() / 2);
            }

            os_buffers_.resize(num_channels);
            for (auto &buffer: os_buffers_) {
                buffer.resize(max_num_samples << 1);
            }

            os_pointers_.resize(num_channels);
            for (size_t chan = 0; chan < num_channels; ++chan) {
                os_pointers_[chan] = os_buffers_[chan].data();
            }
            reset();
        }

        void reset() {
            down_center_pos_ = 0;
            for (auto &d: up_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
            for (auto &d: down_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
            for (auto &d: down_center_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
        }

        [[nodiscard]] size_t getLatency() const { return latency_; }

        template<bool use_simd = false>
        void upsample(std::span<FloatType *> buffer, const size_t num_samples) {
            const auto symmetric_size = up_coeff_.size() >> 1;
            const auto symmetric_shift = up_coeff_.size() - 1;

            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto delay_line = up_delay_lines_[chan].data();
                auto os_data = os_buffers_[chan].data();
                auto chan_data = buffer[chan];
                for (size_t i = 0; i < num_samples; ++i) {
                    os_data[i << 1] = delay_line[up_coeff_center_pos_ + i] * up_coeff_center_;
                    delay_line[up_coeff_.size() + i] = chan_data[i];
                    if constexpr (use_simd) {
                        auto v = kfr::make_univector(&delay_line[i + 1], up_coeff_.size());
                        os_data[(i << 1) + 1] = kfr::dotproduct(v, up_coeff_);
                    } else {
                        FloatType output{FloatType(0)};
                        const auto shifted_delay_line = &delay_line[i + 1];
                        for (size_t k = 0; k < symmetric_size; ++k) {
                            output += (shifted_delay_line[k] + shifted_delay_line[symmetric_shift - k]) * up_coeff_[k];
                        }
                        os_data[(i << 1) + 1] = output;
                    }
                }
            }

            const auto memmove_size = up_coeff_.size() * sizeof(FloatType);
            for (auto &delay_line: up_delay_lines_) {
                std::memmove(delay_line.data(), delay_line.data() + num_samples, memmove_size);
            }
        }

        template<bool use_simd = false>
        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            const auto symmetric_size = down_coeff_.size() >> 1;
            const auto symmetric_shift = down_coeff_.size() - 1;

            size_t center_pos{down_center_pos_};
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto delay_line = down_delay_lines_[chan].data();
                auto center_delay_line = down_center_delay_lines_[chan].data();
                auto os_data = os_buffers_[chan].data();
                auto chan_data = buffer[chan];
                center_pos = down_center_pos_;
                for (size_t i = 0; i < num_samples; ++i) {
                    if constexpr (use_simd) {
                        auto v = kfr::make_univector(&delay_line[i], down_coeff_.size());
                        FloatType output = center_delay_line[center_pos] * down_coeff_center_;
                        chan_data[i] = output + kfr::dotproduct(v, down_coeff_);
                    } else {
                        FloatType output = center_delay_line[center_pos] * down_coeff_center_;
                        const auto shifted_delay_line = &delay_line[i];
                        for (size_t k = 0; k < symmetric_size; ++k) {
                            output += (shifted_delay_line[k] + shifted_delay_line[symmetric_shift - k]) * down_coeff_[
                                k];
                        }
                        chan_data[i] = output;
                    }
                    delay_line[down_coeff_.size() + i] = os_data[(i << 1) + 1];
                    center_delay_line[center_pos] = os_data[i << 1];

                    center_pos = (center_pos == 0) ? down_center_delay_lines_[chan].size() - 1 : center_pos - 1;
                }
            }

            down_center_pos_ = center_pos;

            const auto memmove_size = down_coeff_.size() * sizeof(FloatType);
            for (auto &delay_line: down_delay_lines_) {
                std::memmove(delay_line.data(), delay_line.data() + num_samples, memmove_size);
            }
        }

        std::vector<std::vector<FloatType> > &getOSBuffer() { return os_buffers_; }

        std::vector<FloatType *> &getOSPointer() { return os_pointers_; }

    private:
        kfr::univector<FloatType> up_coeff_{};
        FloatType up_coeff_center_{FloatType(0)};
        size_t up_coeff_center_pos_{0};
        std::vector<kfr::univector<FloatType> > up_delay_lines_{};

        kfr::univector<FloatType> down_coeff_{};
        FloatType down_coeff_center_{FloatType(0)};
        std::vector<kfr::univector<FloatType> > down_delay_lines_{};
        size_t down_center_pos_{0};
        std::vector<kfr::univector<FloatType> > down_center_delay_lines_{};

        size_t latency_{0};

        std::vector<std::vector<FloatType> > os_buffers_{};
        std::vector<FloatType *> os_pointers_{};
    };
}
