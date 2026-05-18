// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <algorithm>
#include "fifo_base.hpp"

namespace zldsp::container {
    /**
     * an abstract FIFO for single-thread one producer and multiple consumers.
     */
    class SingleThreadMulticastFIFO {
    public:
        explicit SingleThreadMulticastFIFO(const int capacity = 0) :
            capacity_(capacity),
            tail_(0) {
        }

        ~SingleThreadMulticastFIFO() = default;

        void setCapacity(const int capacity) {
            capacity_ = capacity;
            tail_ = 0;
            std::ranges::fill(reader_heads_, 0);
        }

        [[nodiscard]] int getCapacity() const { return capacity_; }

        /**
         * register a consumer
         * @return consumer ID
         */
        size_t addConsumer() {
            for (size_t i = 0; i < active_readers_.size(); ++i) {
                if (!active_readers_[i]) {
                    active_readers_[i] = true;
                    reader_heads_[i] = tail_;
                    return i;
                }
            }
            reader_heads_.push_back(tail_);
            active_readers_.push_back(true);
            return reader_heads_.size() - 1;
        }

        /**
         * remove a consumer
         * @param consumer_id
         */
        void removeConsumer(const size_t consumer_id) {
            if (isConsumerValid(consumer_id)) {
                active_readers_[consumer_id] = false;
            }
        }

        [[nodiscard]] int getNumFree() const {
            const int slowest_head = getSlowestReaderHead();
            if (slowest_head > tail_) {
                return slowest_head - tail_ - 1;
            }
            return capacity_ - tail_ + slowest_head - 1;
        }

        [[nodiscard]] FIFORange prepareToWrite(const int num_to_write) const {
            FIFORange range;
            range.start_index1 = tail_;
            range.block_size1 = std::min(num_to_write, capacity_ - tail_);

            if (num_to_write > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_write - range.block_size1;
            } else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
            return range;
        }

        void finishWrite(const int num_written) {
            tail_ = (tail_ + num_written) % capacity_;
        }

        int getNumReady(const size_t consumer_id) const {
            const int head = reader_heads_[consumer_id];
            if (tail_ >= head) {
                return tail_ - head;
            } else {
                return capacity_ - head + tail_;
            }
        }

        [[nodiscard]] FIFORange prepareToRead(const size_t consumer_id, const int num_to_read) const {
            FIFORange range;
            const int head = reader_heads_[consumer_id];

            range.start_index1 = head;
            range.block_size1 = std::min(num_to_read, capacity_ - head);

            if (num_to_read > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_read - range.block_size1;
            } else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
            return range;
        }

        void finishRead(const size_t consumer_id, const int num_read) {
            reader_heads_[consumer_id] = (reader_heads_[consumer_id] + num_read) % capacity_;
        }

    private:
        [[nodiscard]] bool isConsumerValid(const size_t id) const {
            return id < active_readers_.size() && active_readers_[id];
        }

        [[nodiscard]] int getSlowestReaderHead() const {
            int max_ready = -1;
            int slowest_head = tail_;

            for (size_t i = 0; i < active_readers_.size(); ++i) {
                if (active_readers_[i]) {
                    const int h = reader_heads_[i];

                    int ready;
                    if (tail_ >= h) {
                        ready = tail_ - h;
                    } else {
                        ready = capacity_ - h + tail_;
                    }

                    if (ready > max_ready) {
                        max_ready = ready;
                        slowest_head = h;
                    }
                }
            }
            return slowest_head;
        }

        int capacity_;
        int tail_;
        std::vector<int> reader_heads_;
        std::vector<bool> active_readers_;
    };
}
