// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <algorithm>
#include "fifo_base.hpp"

namespace zldsp::container {
    /**
     * an abstract FIFO that can be used by one producer and one consumer
     */
    class AbstractFIFO {
    public:
        explicit AbstractFIFO(const int capacity = 0)
            : capacity_(capacity),
              head_(0),
              tail_(0) {
        }

        ~AbstractFIFO() = default;

        void setCapacity(const int capacity) {
            capacity_ = capacity;
            head_.store(0);
            tail_.store(0);
        }

        int getCapacity() const { return capacity_; }

        /**
         * get the number of elements that can be read
         * @return
         */
        int getNumReady() const {
            const int current_head = head_.load(std::memory_order::relaxed);
            const int current_tail = tail_.load(std::memory_order::acquire);
            if (current_tail >= current_head) {
                return current_tail - current_head;
            }
            return capacity_ - current_head + current_tail;
        }

        /**
         * get the number of elements that can be written
         * @return
         */
        int getNumFree() const {
            const int current_head = head_.load(std::memory_order::acquire);
            const int current_tail = tail_.load(std::memory_order::relaxed);
            if (current_head > current_tail) {
                return current_head - current_tail - 1;
            }
            return capacity_ - current_tail + current_head - 1;
        }

        FIFORange prepareToWrite(const int num_to_write) const {
            FIFORange range;
            const int current_tail = tail_.load(std::memory_order::relaxed);

            range.block_size1 = std::min(num_to_write, capacity_ - current_tail);
            range.start_index1 = current_tail;

            if (num_to_write > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_write - range.block_size1;
            }
            else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
            return range;
        }

        void finishWrite(const int num_written) {
            if (num_written > 0) {
                const int current_tail = tail_.load(std::memory_order::relaxed);
                const int new_tail = (current_tail + num_written) % capacity_;
                tail_.store(new_tail, std::memory_order::release);
            }
        }

        FIFORange prepareToRead(const int num_to_read) const {
            FIFORange range;
            const int current_head = head_.load(std::memory_order::relaxed);

            range.block_size1 = std::min(num_to_read, capacity_ - current_head);
            range.start_index1 = current_head;

            if (num_to_read > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_read - range.block_size1;
            }
            else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
            return range;
        }

        void finishRead(const int num_read) {
            if (num_read > 0) {
                const int current_head = head_.load(std::memory_order::relaxed);
                const int new_head = (current_head + num_read) % capacity_;
                head_.store(new_head, std::memory_order::release);
            }
        }

    private:
        int capacity_;
        std::atomic<int> head_;
        std::atomic<int> tail_;
    };
}
