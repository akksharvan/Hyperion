#pragma once

#include <cstdint>
#include <limits>

namespace Hyperion {
    using OrderId = uint64_t;
    using Price = uint64_t;
    using Quantity = uint32_t;
    using Token = uint64_t;

    constexpr Price INVALID_PRICE = std::numeric_limits<Price>::max();
    constexpr OrderId INVALID_ORDER_ID = 0;

    enum class Side : uint8_t {
        Buy = 'B',
        Sell = 'S',
        Unknown = 0
    };

    enum class OrderType : uint8_t {
        Limit = 'L',
        Market = 'M'
    };

    struct alignas(64) Order {
        OrderId id = INVALID_ORDER_ID;
        Price price = INVALID_PRICE;

        Quantity qty = 0;
        Side side = Side::Unknown;

        uint64_t timestamp = 0;

        Order* next = nullptr;
        Order* prev = nullptr;

        void reset() {
            id = INVALID_ORDER_ID;
            price = INVALID_PRICE;

            qty = 0;
            side = Side::Unknown;

            timestamp = 0;

            next = nullptr;
            prev = nullptr;
        }
    };
}