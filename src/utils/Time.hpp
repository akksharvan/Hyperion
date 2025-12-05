#pragma once

#include <cstdint>

namespace Hyperion::Utils {
    [[nodiscard]] __attribute__((always_inline))
    inline uint64_t get_cycle_count() {
        #if defined(__aarch64__)
            uint64_t val;
            asm volatile("mrs %0, cntvct_el0" : "=r"(val));
            return val;
        #elif defined(__x86__)
            uin32_t lo, hi;
            asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
            return ((uint64_t) hi << 32) | lo;
        #else
            return 0;
        #endif
    }
}