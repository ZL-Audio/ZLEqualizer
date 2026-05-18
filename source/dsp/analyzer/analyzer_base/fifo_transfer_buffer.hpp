// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <vector>
#include <cstring>
#include <algorithm>
#include "../../container/fifo/abstract_fifo.hpp"
#include "../../container/fifo/single_thread_multicast_fifo.hpp"

namespace zldsp::analyzer {
    /**
     * a transfer buffer which pulls data from a sender FIFO and pushes data into a multicast FIFO
     * @tparam kNum the number of analyzers
     */
    template <size_t kNum>
    class FIFOTransferBuffer {
    public:
        explicit FIFOTransferBuffer() = default;

        /**
         *
         * @param sample_rate
         * @param num_channels
         * @param fifo_size_second
         */
        void prepare(const double sample_rate,
                     const size_t max_num_samples,
                     const std::array<size_t, kNum> num_channels,
                     const double fifo_size_second) {
            sample_rate_ = sample_rate;
            max_num_samples_ = max_num_samples;
            const auto fifo_size = static_cast<size_t>(std::round(sample_rate * fifo_size_second));
            for (size_t i = 0; i < kNum; ++i) {
                sample_fifos_[i].resize(num_channels[i]);
                for (size_t chan = 0; chan < num_channels[i]; ++chan) {
                    sample_fifos_[i][chan].resize(fifo_size);
                    std::fill(sample_fifos_[i][chan].begin(), sample_fifos_[i][chan].end(), 0.f);
                }
            }
            multicast_fifo_.setCapacity(static_cast<int>(fifo_size));
        }

        /**
         *
         * @param sender_fifo the abstract fifo of the sender
         * @param sender_sample_fifos the sample fifos of the sender
         */
        void processTransfer(zldsp::container::AbstractFIFO& sender_fifo,
                             std::array<std::vector<std::vector<float>>, kNum>& sender_sample_fifos) {
            const int num_ready = sender_fifo.getNumReady();
            const int num_free = multicast_fifo_.getNumFree();
            const int num_to_transfer = std::min(num_ready, num_free);

            if (num_to_transfer <= 0) {
                return;
            }

            const auto src_range = sender_fifo.prepareToRead(num_to_transfer);
            const auto dst_range = multicast_fifo_.prepareToWrite(num_to_transfer);

            struct CopyRegion {
                int src_idx;
                int dst_idx;
                int length;
            };

            std::array<CopyRegion, 3> regions;
            size_t region_count = 0;

            int src_current_idx = src_range.start_index1;
            int src_rem_in_block = src_range.block_size1;
            bool src_in_second_block = false;

            int dst_current_idx = dst_range.start_index1;
            int dst_rem_in_block = dst_range.block_size1;
            bool dst_in_second_block = false;

            int remaining = num_to_transfer;

            while (remaining > 0) {
                int amount = std::min(src_rem_in_block, dst_rem_in_block);
                regions[region_count] = {src_current_idx, dst_current_idx, amount};
                region_count += 1;
                remaining -= amount;

                src_current_idx += amount;
                src_rem_in_block -= amount;
                if (src_rem_in_block == 0 && !src_in_second_block) {
                    src_current_idx = src_range.start_index2;
                    src_rem_in_block = src_range.block_size2;
                    src_in_second_block = true;
                }

                dst_current_idx += amount;
                dst_rem_in_block -= amount;
                if (dst_rem_in_block == 0 && !dst_in_second_block) {
                    dst_current_idx = dst_range.start_index2;
                    dst_rem_in_block = dst_range.block_size2;
                    dst_in_second_block = true;
                }
            }

            for (size_t k = 0; k < kNum; ++k) {
                const size_t num_channels = std::min(sender_sample_fifos[k].size(), sample_fifos_[k].size());
                for (size_t ch = 0; ch < num_channels; ++ch) {
                    const float* src_ptr = sender_sample_fifos[k][ch].data();
                    float* dst_ptr = sample_fifos_[k][ch].data();
                    for (size_t r = 0; r < region_count; ++r) {
                        const auto& reg = regions[r];
                        std::memcpy(dst_ptr + static_cast<size_t>(reg.dst_idx),
                                    src_ptr + static_cast<size_t>(reg.src_idx),
                                    static_cast<size_t>(reg.length) * sizeof(float));
                    }
                }
            }

            sender_fifo.finishRead(num_to_transfer);
            multicast_fifo_.finishWrite(num_to_transfer);
        }

        zldsp::container::SingleThreadMulticastFIFO& getMulticastFIFO() {
            return multicast_fifo_;
        }

        std::array<std::vector<std::vector<float>>, kNum>& getSampleFIFOs() {
            return sample_fifos_;
        }

        [[nodiscard]] double getSampleRate() const {
            return sample_rate_;
        }

        [[nodiscard]] size_t getMaxNumSamples() const {
            return max_num_samples_;
        }

    private:
        zldsp::container::SingleThreadMulticastFIFO multicast_fifo_;
        std::array<std::vector<std::vector<float>>, kNum> sample_fifos_;

        double sample_rate_{0};
        size_t max_num_samples_{0};
    };
}
