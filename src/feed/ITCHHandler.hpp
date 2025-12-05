#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include "utils/Endian.hpp"
#include "core/Types.hpp"

namespace Hyperion {
    constexpr char MSG_ADD_ORDER = 'A';
    constexpr char MSG_SYSTEM_EVENT = 'S';

    #pragma pack(push, 1)

    struct ITCHMsgHeader {
        uint16_t length;
        char ms_type;
    };

    struct MsgAddOrder {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint8_t timestamp[6];
        uint64_t order_ref;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
    };

    #pragma pack(pop)

    template<typename BufferType>
    class ITCHHandler {
        private:
            BufferType& buffer_;
            char* data_;
            size_t size_;
            // size_t current_offset_;

            // void print_order(const MsgAddOrder* msg) {
            //     uint64_t ts = Utils::parse_timestamp48(msg -> timestamp);
            //     uint64_t ref = Utils::bswap64(msg -> order_ref);
            //     uint32_t shares = Utils::bswap32(msg -> shares);
            //     uint32_t price = Utils::bswap32(msg -> price);

            //     std::string symbol(msg -> stock, 8);

            //     std::cout << "[ITCH] ADD ORDER | TS: " << ts
            //               << " | Ref: " << ref
            //               << " | SIDE: " << msg -> buy_sell_indicator
            //               << " | SHARES: " << shares
            //               << " | SYMBOL: " << symbol
            //               << " | PRICE: " << price
            //               << "\n";
            // }

        public:
            explicit ITCHHandler(BufferType& buffer) : buffer_(buffer), data_(nullptr), size_(0) {}
            // ITCHHandler() : data_(nullptr), size_(0), current_offset_(0) {}

            ~ITCHHandler() {
                if (data_ && data_ != MAP_FAILED) {
                    munmap(data_, size_);
                }
            }

            bool load_file(const std::string& filepath) {
                int fd = open(filepath.c_str(), O_RDONLY);

                if (fd == -1) {
                    std::cerr << "Error opening file: " << filepath << "\n";
                    return false;
                }

                struct stat sb;

                if (fstat(fd, &sb) == -1) {
                    close(fd);
                    return false;
                }

                size_ = sb.st_size;

                #ifdef __APPLE__
                data_ = static_cast<char*>(mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0));
                #else
                data_ = static_cast<char*>(mmap(nullptr, size_, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0));
                #endif

                close(fd);

                if (data_ == MAP_FAILED) {
                    std::cerr << "mmap failed\n";
                    return false;
                }

                std::cout << "Mapped " << size_ / 1024 / 1024 << " MB of market data.\n";
                return true;
            }

            void process() {
                if (!data_) return;

                const char* ptr = data_;
                const char* end = data_ + size_;

                // size_t msg_count = 0;

                while (ptr < end) {
                    if (ptr + 2 > end) break;

                    uint16_t msg_len = Utils::bswap16(*reinterpret_cast<const uint16_t*>(ptr));
                    ptr += 2;

                    if (ptr + msg_len > end) break;

                    char msg_type = *ptr;

                    // switch (msg_type) {
                    //     case MSG_ADD_ORDER: {
                    //         const auto& msg = reinterpret_cast<const MsgAddOrder*>(ptr + 1);

                    //         if (msg_count < 5) {
                    //             print_order(msg);
                    //         }

                    //         break;
                    //     }
                    //     default:
                    //         break;
                    // }

                    // ptr += msg_len;
                    // msg_count++;

                    if (msg_type == MSG_ADD_ORDER) {
                        const auto* msg = reinterpret_cast<const MsgAddOrder*>(ptr + 1);

                        Order order;
                        order.id = Utils::bswap64(msg -> order_ref);
                        order.price = Utils::bswap32(msg -> price);
                        order.qty = Utils::bswap32(msg -> shares);
                        order.side = (msg -> buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
                        order.timestamp = Utils::parse_timestamp48(msg -> timestamp);

                        while (!buffer_.push(order)) {
                            std::this_thread::yield();
                        }
                    }

                    ptr += msg_len;
                }

                // std::cout << "Processed " << msg_count << " messages.\n";
            }
    };
}