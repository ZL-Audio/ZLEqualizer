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

namespace zlchore::thread {
    class Notifier {
    public:
        explicit Notifier(const bool initial_state = false) :
            flag_(initial_state) {
        }

        void signal() {
            flag_.store(true, std::memory_order_release);
        }

        bool check() {
            if (!flag_.load(std::memory_order_relaxed)) {
                return false;
            }
            return flag_.exchange(false, std::memory_order_acquire);
        }

    private:
        std::atomic<bool> flag_{false};
    };
}
