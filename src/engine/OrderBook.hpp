#pragma once

#include <iostream>
#include "core/Types.hpp"
#include "core/ObjectPool.hpp"

namespace Hyperion {
    template<size_t PoolSize>
    class OrderBook {
        private:
            ObjectPool<Order, PoolSize> pool_;
            uint64_t orders_processed_;
            Price last_price_ = 0;
        
        public:
            OrderBook() : orders_processed_(0) {}

            void process(const Order& incoming_order) {
                Order* node = pool_.allocate();

                if (!node) [[unlikely]] {
                    std::cerr << "CRITICAL: Object Pool Exhausted!\n";
                    return;
                }

                *node = incoming_order;
                orders_processed_++;
                last_price_ = node -> price;
            }

            [[nodiscard]] uint64_t get_processed_count() const { return orders_processed_; }
            [[nodiscard]] Price get_last_price() const { return last_price_; }
    };
}