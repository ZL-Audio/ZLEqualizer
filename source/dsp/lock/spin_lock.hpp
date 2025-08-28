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

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace zldsp::lock {
    class SpinLock {
    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;

    public:
        void lock() noexcept {
            while (flag.test_and_set(std::memory_order_acquire)) {
#if defined(_MSC_VER) && !defined(__clang__)
                _mm_pause();
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
                __builtin_ia32_pause();
#elif defined(__aarch64__)
                asm volatile("yield");
#endif
#endif
            }
        }

        bool try_lock() noexcept [[clang::nonblocking]] {
            return !flag.test_and_set(std::memory_order_acquire);
        }

        void unlock() noexcept [[clang::nonblocking]] {
            flag.clear(std::memory_order_release);
        }
    };
}
