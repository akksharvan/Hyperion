#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <new>
#include <type_traits>

namespace Hyperion {
    #ifdef __cpp_lib_hardware_inference_size
        constexpr size_t CACHE_LINE_SIZE = std::max(std::hardware_constructive_interference_size, size_t(128));
    #else
        constexpr size_t CACHE_LINE_SIZE = 128;
    #endif

    template<typename T, size_t Size>
    class alignas(CACHE_LINE_SIZE) RingBuffer {
        static_assert((Size != 0) && ((Size & (Size - 1)) == 0), "Size must be a power of two");

        private:
            static constexpr size_t mask_ = Size - 1;

            alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> head_;
            alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> tail_;
            alignas(CACHE_LINE_SIZE) std::array<T, Size> buffer_;

        public:
            RingBuffer() : head_(0), tail_(0) {}

            bool push(const T& item) {
                const auto current_head = head_.load(std::memory_order_relaxed);
                const auto current_tail = tail_.load(std::memory_order_acquire);

                if (current_head - current_tail >= Size) {
                    return false;
                }

                buffer_[current_head & mask_] = item;
                head_.store(current_head + 1, std::memory_order_release);

                return true;
            }

            bool pop(T& item) {
                const auto current_tail = tail_.load(std::memory_order_relaxed);
                const auto current_head = head_.load(std::memory_order_acquire);

                if (current_tail == current_head) {
                    return false;
                }

                item = buffer_[current_tail & mask_];
                tail_.store(current_tail + 1, std::memory_order_release);

                return true;
            }
    };
}