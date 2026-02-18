// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <thread>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace zldsp::lock {
    /**
     * a spin lock which has non-blocking try_lock and unlock methods
     */
    class SpinLock {
    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;

        static inline void cpu_relax() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
            _mm_pause();
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
            __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
            asm volatile("yield");
#endif
#endif
        }

    public:
        SpinLock() = default;

        SpinLock(const SpinLock&) = delete;

        SpinLock& operator=(const SpinLock&) = delete;

        /**
         * acquire the lock
         */
        void lock() noexcept {
            if (!flag.test_and_set(std::memory_order_acquire)) {
                return;
            }
            int spin_count = 0;
            int count_start = 0;
            while (true) {
                if (count_start < 128) {
                    while (flag.test(std::memory_order_relaxed)) {
                        if (spin_count < 128) {
                            cpu_relax();
                            spin_count += 1;
                        } else {
                            std::this_thread::yield();
                        }
                    }
                    count_start += 16;
                    spin_count = count_start;
                } else {
                    while (flag.test(std::memory_order_relaxed)) {
                        std::this_thread::yield();
                    }
                }
                if (!flag.test_and_set(std::memory_order_acquire)) {
                    return;
                }
            }
        }

        /**
         * try to acquire the lock
         * @return true if the locked is acquired
         */
        bool try_lock() noexcept [[clang::nonblocking]] {
            return !flag.test_and_set(std::memory_order_acquire);
        }

        /**
         * release the lock
         */
        void unlock() noexcept [[clang::nonblocking]] {
            flag.clear(std::memory_order_release);
        }
    };
}
