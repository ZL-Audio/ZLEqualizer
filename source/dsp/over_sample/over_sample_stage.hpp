// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <numbers>

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

        void prepare(const size_t num_channels, const size_t num_samples) {
            up_delay_lines_.resize(num_channels);
            for (auto &d : up_delay_lines_) {
                d.resize(up_coeff_.size());
            }
            down_delay_lines_.resize(num_channels);
            for (auto &d : down_delay_lines_) {
                d.resize(down_coeff_.size());
            }
            down_center_delay_lines_.resize(num_channels);
            for (auto &d : down_center_delay_lines_) {
                d.resize(down_coeff_.size() / 2);
            }
            os_buffers_.resize(num_channels);
            for (auto &buffer:os_buffers_) {
                buffer.resize(num_samples << 1);
            }
            os_pointers_.resize(num_channels);
            for (size_t chan = 0; chan < num_channels; ++chan) {
                os_pointers_[chan] = os_buffers_[chan].data();
            }
            reset();
        }

        void reset() {
            up_cursor_ = 0;
            down_cursor_ = 0;
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

        [[nodiscard]] size_t getLatency() const {
            return latency_;
        }

        /**
         * process samples up
         * @tparam UseSIMD use SIMD dot-product, faster for large filters
         * @param buffer input samples
         * @param num_samples
         */
        template<bool UseSIMD = true>
        void upsample(std::span<FloatType *> buffer, const size_t num_samples) {
            const auto symmetric_size = up_delay_lines_[0].size() >> 1;
            const auto symmetric_shift = up_delay_lines_[0].size() - 1;
            const auto memmove_size = (up_delay_lines_[0].size() - 1) * sizeof(FloatType);
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto &delay_line{up_delay_lines_[chan]};
                auto chan_data{buffer[chan]};
                auto os_data{os_buffers_[chan].data()};
                for (size_t i = 0; i < num_samples; ++i) {
                    os_data[i << 1] = delay_line[up_coeff_center_pos_] * up_coeff_center_;
                    std::memmove(delay_line.data(), delay_line.data() + 1, memmove_size);
                    delay_line[delay_line.size() - 1] = chan_data[i];
                    if constexpr (UseSIMD) {
                        os_data[(i << 1) + 1] = kfr::dotproduct(up_coeff_, delay_line);
                    } else {
                        FloatType output{FloatType(0)};
                        for (size_t k = 0; k < symmetric_size; ++k) {
                            output += (delay_line[k] + delay_line[symmetric_shift - k]) * up_coeff_[k];
                        }
                        os_data[(i << 1) + 1] = output;
                    }
                }
            }
        }

        /**
         * process samples down
         * @tparam UseSIMD use SIMD dot-product, faster for large filters
         * @param buffer output samples
         * @param num_samples
         */
        template<bool UseSIMD = true>
        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            const auto symmetric_size = down_delay_lines_[0].size() >> 1;
            const auto symmetric_shift = down_delay_lines_[0].size() - 1;
            const auto memmove_size = (down_delay_lines_[0].size() - 1) * sizeof(FloatType);
            size_t center_pos{0};
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto &delay_line{down_delay_lines_[chan]};
                auto chan_data{buffer[chan]};
                auto os_data{os_buffers_[chan].data()};
                center_pos = down_center_pos_;
                auto &center_delay_line {down_center_delay_lines_[chan]};
                for (size_t i = 0; i < num_samples; ++i) {
                    FloatType output = center_delay_line[center_pos] * down_coeff_center_;
                    if constexpr (UseSIMD) {
                        output += kfr::dotproduct(delay_line, down_coeff_);
                    } else {
                        for (size_t k = 0; k < symmetric_size; ++k) {
                            output += (delay_line[k] + delay_line[symmetric_shift - k]) * down_coeff_[k];
                        }
                    }
                    chan_data[i] = output;
                    std::memmove(delay_line.data(), delay_line.data() + 1, memmove_size);
                    delay_line.back() = os_data[(i << 1) + 1];
                    center_delay_line[center_pos] = os_data[(i << 1)];
                    center_pos = center_pos == 0 ? center_delay_line.size() - 1 : center_pos - 1;
                }
            }
            down_center_pos_ = center_pos;
        }

        std::vector<std::vector<FloatType> > &getOSBuffer() {
            return os_buffers_;
        }

        std::vector<FloatType *> &getOSPointer() {
            return os_pointers_;
        }

    private:
        size_t up_cursor_{0};
        kfr::univector<FloatType> up_coeff_{};
        FloatType up_coeff_center_{FloatType(0)};
        size_t up_coeff_center_pos_{0};
        std::vector<kfr::univector<FloatType> > up_delay_lines_{};

        size_t down_cursor_{0};
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
