#pragma once

#include <vector>
#include <cassert>
#include <concepts>
#include <cstdint>

namespace Hyperion {
    template<typename T>
    concept Poolable = std::is_default_constructible_v<T>;

    template<typename T, size_t Size>
    requires Poolable<T>
    class ObjectPool {
        private:
                std::vector<T> store_;
                std::vector<size_t> free_indices_;
                size_t next_free_index_;

        public:
            ObjectPool() : next_free_index_(0) {
                store_.resize(Size);
                free_indices_.reserve(Size);

                for (size_t i = 0; i < Size; i++) {
                    free_indices_.push_back(i);
                }

                next_free_index_ = Size;
            }

            ObjectPool(const ObjectPool&) = delete;
            ObjectPool& operator=(const ObjectPool&) = delete;

            [[nodiscard]] T* allocate() {
                if (next_free_index_ == 0) [[unlikely]] {
                    return nullptr;
                }

                size_t slot_index = free_indices_[--next_free_index_];
                return &store_[slot_index];
            }

            void deallocate(T* ptr) {
                const size_t index = ptr - store_.data();
                assert(index < Size && "Pointer does not belong to this pool!");

                if constexpr (requires { ptr -> reset(); })  {
                    ptr -> reset();
                }

                free_indices_[next_free_index_++] = index;
            }

            [[nodiscard]] size_t capacity() const { return Size; }
            [[nodiscard]] size_t available() const { return next_free_index_; }
    };
}