// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlpanel {
    template <typename C>
    class TriBuffer {
    public:
        TriBuffer() = default;

        /**
         * pull new object before reading
         * @return whether the object has been updated
         */
        bool pull() {
            if (new_frame_ready_.exchange(false, std::memory_order_acquire)) {
                read_idx_ = clean_idx_.exchange(read_idx_, std::memory_order_acq_rel);
                return true;
            }
            return false;
        }

        /**
         * get the reader object
         * @return
         */
        const C& getReader() const {
            return buffer_[static_cast<size_t>(read_idx_)];
        }

        /**
         * get the writer object
         * @return
         */
        C& getWriter() {
            return buffer_[static_cast<size_t>(write_idx_)];
        }

        /**
         * publish new object after writing
         */
        void publish() {
            write_idx_ = clean_idx_.exchange(write_idx_, std::memory_order_acq_rel);
            new_frame_ready_.store(true, std::memory_order_release);
        }

        std::array<C, 3>& getBuffer() {
            return buffer_;
        }

    private:
        std::array<C, 3> buffer_;

        alignas(64) std::atomic<int> clean_idx_{2};
        std::atomic<bool> new_frame_ready_{false};

        alignas(64) int read_idx_{0};
        alignas(64) int write_idx_{1};
    };
}
