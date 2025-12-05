#pragma once

#include <cstdint>
#include <bit>

namespace Hyperion::Utils {
    [[nodiscard]] __attribute__((always_inline))
    inline uint16_t bswap16(uint16_t v) {
        return __builtin_bswap16(v);
    }

    [[nodiscard]] __attribute__((always_inline))
    inline uint32_t bswap32(uint32_t v) {
        return __builtin_bswap32(v);
    }

    [[nodiscard]] __attribute__((always_inline))
    inline uint64_t bswap64(uint64_t v) {
        return __builtin_bswap64(v);
    }

    [[nodiscard]] __attribute__((always_inline))
    inline uint64_t parse_timestamp48(const uint8_t* ptr) {
        uint64_t high = bswap16(*reinterpret_cast<const uint16_t*>(ptr));
        uint32_t low = bswap32(*reinterpret_cast<const uint32_t*>(ptr + 2));
        return (high << 32) | low;
    }
}