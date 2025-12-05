#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem>

#include "core/Types.hpp"
#include "core/ObjectPool.hpp"
#include "core/RingBuffer.hpp"
#include "feed/ITCHHandler.hpp"
#include "engine/OrderBook.hpp"
#include "utils/Endian.hpp"
#include "utils/Thread.hpp"
#include "utils/Time.hpp"

using namespace Hyperion;

constexpr size_t RB_SIZE = 4096;
constexpr size_t OP_SIZE = 1'000'000'000;

RingBuffer<Order, RB_SIZE> ring_buffer;
OrderBook<OP_SIZE> order_book;

// void create_dummy_itch_file(const std::string& filename, int count) {
//     std::ofstream ofs(filename, std::ios::binary);

//     for (int i = 0; i < count; i++) {
//         uint16_t len = Utils::bswap16(36);
        
//         ofs.write(reinterpret_cast<char*>(&len), 2);
//         ofs.put('A');

//         uint16_t loc = Utils::bswap16(1);
//         uint16_t trk = Utils::bswap16(1);

//         ofs.write(reinterpret_cast<char*>(&loc), 2);
//         ofs.write(reinterpret_cast<char*>(&trk), 2);

//         uint16_t ts_high = 0;
//         uint32_t ts_low = 0;

//         ofs.write(reinterpret_cast<char*>(&ts_high), 2);
//         ofs.write(reinterpret_cast<char*>(&ts_low), 4);

//         uint64_t ref = Utils::bswap64(1000 + i);
//         ofs.write(reinterpret_cast<char*>(&ref), 8);

//         ofs.put('B');

//         uint32_t qty = Utils::bswap32(100);
//         ofs.write(reinterpret_cast<char*>(&qty), 4);

//         ofs.write("AAPL    ", 8);

//         uint32_t px = Utils::bswap32(150000 + i);
//         ofs.write(reinterpret_cast<char*>(&px), 4);
//     }

//     ofs.close();
// }

void feed_thread_func(const std::string& file) {
    Utils::pin_thread(1);
    ITCHHandler<RingBuffer<Order, RB_SIZE>> handler(ring_buffer);

    if (handler.load_file(file)) {
        handler.process();
    }

    Order poison;
    poison.id = INVALID_ORDER_ID;

    while (!ring_buffer.push(poison)) {
        std::this_thread::yield();
    }
}

void engine_thread_func() {
    Utils::pin_thread(2);
    Order order;

    while (true) {
        if (ring_buffer.pop(order)) {
            if (order.id == INVALID_ORDER_ID) break;
            order_book.process(order);
        } else {
            std::this_thread::yield();
        }
    }
}

// constexpr size_t RB_SIZE = 1024;
// RingBuffer<int, RB_SIZE> ring_buffer;

// void producer_thread(int count) {
//     for (int i = 0; i < count; i++) {
//         while (!ring_buffer.push(i)) {
//             std::this_thread::yield();
//         }
//     }
// }

// void consumer_thread(int count) {
//     int received = 0;
//     int val;

//     while (received < count) {
//         if (ring_buffer.pop(val)) {
//             if (val != received) {
//                 std::cerr << "ERROR: Sequence mismatch! Expected " << received << " got " << val << "\n";
//                 std::exit(1);
//             }

//             received++;
//         } else {
//             std::this_thread::yield();
//         }
//     }
// }

// void create_dummy_itch_file(const std::string& filename) {
//     std::ofstream ofs(filename, std::ios::binary);

//     uint16_t stock_locate = 1;
//     uint16_t tracking = 1;
//     uint64_t timestamp = 0x112233445566;
//     uint64_t ref = 123456789;
//     char side = 'B';
//     uint32_t shares = 100;
//     char stock[8] = {'A', 'A', 'P', 'L', ' ', ' ', ' ', ' '};
//     uint32_t price = 150000;

//     std::vector<uint8_t> buffer;
//     uint16_t len = Utils::bswap16(36);

//     buffer.push_back(len * 0xFF);
//     buffer.push_back(len >> 8);

//     ofs.write(reinterpret_cast<char*>(&len), 2);
//     ofs.put('A');

//     uint16_t be_loc = Utils::bswap16(stock_locate);
//     ofs.write(reinterpret_cast<char*>(&be_loc), 2);

//     uint16_t be_trk = Utils::bswap16(tracking);
//     ofs.write(reinterpret_cast<char*>(&be_trk), 2);

//     uint16_t ts_high = Utils::bswap16(static_cast<uint16_t>((timestamp >> 32) & 0xFFFF));
//     uint32_t ts_low = Utils::bswap32(static_cast<uint32_t>(timestamp & 0xFFFFFFFF));

//     ofs.write(reinterpret_cast<char*>(&ts_high), 2);
//     ofs.write(reinterpret_cast<char*>(&ts_low), 4);

//     uint64_t be_ref = Utils::bswap64(ref);
//     ofs.write(reinterpret_cast<char*>(&be_ref), 8);

//     ofs.put(side);

//     uint32_t be_shares = Utils::bswap32(shares);
//     ofs.write(reinterpret_cast<char*>(&be_shares), 4);

//     ofs.write(stock, 8);

//     uint32_t be_price = Utils::bswap32(price);
//     ofs.write(reinterpret_cast<char*>(&be_price), 4);

//     ofs.close();
//     std::cout << "Generated dummy ITCH file: " << filename << "\n";
// }

int main() {
    // std::cout << "[Hyperion] Phase 1: Core Initialization\n";
    // std::cout << "Order Size: " << sizeof(Order) << " bytes (Aligned: " << alignof(Order) << ")\n";

    // if (sizeof(Order) != 64) {
    //     std::cerr << "WARNING: Order struct is not 64 bytes! Cache alignment may be compromised.\n";
    // }

    // constexpr size_t POOL_SIZE = 1024;
    // ObjectPool<Order, POOL_SIZE> order_pool;

    // std::cout << "Pool Initialized. Capcity: " << order_pool.capacity() << "\n";

    // Order* o1 = order_pool.allocate();
    // Order* o2 = order_pool.allocate();

    // if (o1 && o2) {
    //     o1 -> id = 1001;
    //     o1 -> price = 5000;
    //     o1 -> side = Side::Buy;

    //     std::cout << "Allocated Order 1 ID: " << o1 -> id << " at address " << o1 << "\n";
    //     std::cout << "Allocated Order 2 at address " << o2 << "\n";
    //     std::cout << "Pool Available: " << order_pool.available() << "\n";
    // }

    // order_pool.deallocate(o1);
    // order_pool.deallocate(o2);

    // std::cout << "Orders relased. Pool Available: " << order_pool.available() << "\n";

    // return 0;
    // std::cout << "[Hyperion] Phase 2: Parser Integration\n";

    // const std::string test_file = "test_data.itch";
    // create_dummy_itch_file(test_file);

    // ITCHHandler handler;

    // if (handler.load_file(test_file)) {
    //     handler.process();
    // }

    // return 0;
    // std::cout << "[Hyperion] Phase 3: Ring Buffer Verification\n";
    // std::cout << "Cache Line Size: " << Hyperion::CACHE_LINE_SIZE << " bytes\n";

    // const int MSG_COUNT = 1'000'000;
    // auto start = std::chrono::high_resolution_clock::now();

    // std::thread p(producer_thread, MSG_COUNT);
    // std::thread c(consumer_thread, MSG_COUNT);

    // p.join();
    // c.join();

    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // std::cout << "Successfully passed " << MSG_COUNT << " messages through RingBuffer.\n";
    // std::cout << "Time: " << duration.count() << "ms\n";

    // return 0;
    // std::cout << "[Hyperion] Phase 4: System Integration\n";

    // const std::string test_file = "integration_test.itch";
    // const int MSG_COUNT = 100'000;

    // std::cout << "Generating " << MSG_COUNT << " orders...\n";
    // create_dummy_itch_file(test_file, MSG_COUNT);

    // auto start = std::chrono::high_resolution_clock::now();

    // std::thread producer(feed_thread_func, test_file);
    // std::thread consumer(engine_thread_func);

    // producer.join();
    // consumer.join();

    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // std::cout << "Processed " << order_book.get_processed_count() << " orders.\n";
    // std::cout << "Total Time: " << duration.count() << "ms\n";

    // if (duration.count() > 0) {
    //     double throughput = (double) MSG_COUNT / duration.count() * 1000.0;
    //     std::cout << "Throughput: " << (size_t) throughput << " orders/sec\n";
    // }

    // return 0;
    // std::cout << "[Hyperion] Phase 5: Platform Optimization (M2 Pro)\n";

    // const std::string test_file = "integration_test.itch";
    // const int MSG_COUNT = 1'000'000;

    // std::cout << "Generating " << MSG_COUNT << " orders...\n";
    // create_dummy_itch_file(test_file, MSG_COUNT);
    
    // std::cout << "Starting Engine...\n";
    
    // uint64_t start_cycles = Utils::get_cycle_count();
    // auto start_wall = std::chrono::high_resolution_clock::now();

    // std::thread producer(feed_thread_func, test_file);
    // std::thread consumer(engine_thread_func);

    // producer.join();
    // consumer.join();

    // uint64_t end_cycles = Utils::get_cycle_count();
    // auto end_wall = std::chrono::high_resolution_clock::now();

    // auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_wall - start_wall).count();
    // uint64_t duration_cycles = end_cycles - start_cycles;

    // std::cout << "------------------------------------------------\n";
    // std::cout << "Processed:   " << order_book.get_processed_count() << " orders\n";
    // std::cout << "Wall Time:   " << duration_ms << " ms\n";
    // std::cout << "CPU Cycles:  " << duration_cycles << " (approx)\n";
    
    // if (duration_ms > 0) {
    //     double throughput = (double) MSG_COUNT / duration_ms * 1000.0;
    //     std::cout << "Throughput:  " << (size_t) throughput << " orders/sec\n";
    //     std::cout << "Latency:     " << (double) duration_ms * 1000.0 / MSG_COUNT << " us/order (amortized)\n";
    // }

    // std::cout << "------------------------------------------------\n";
    
    // const std::string test_file = "01302020.NASDAQ_ITCH50";    
    // std::cout << "Starting Engine...\n";
    
    // uint64_t start_cycles = Utils::get_cycle_count();
    // auto start_wall = std::chrono::high_resolution_clock::now();

    // std::thread producer(feed_thread_func, test_file);
    // std::thread consumer(engine_thread_func);

    // producer.join();
    // consumer.join();

    // uint64_t end_cycles = Utils::get_cycle_count();
    // auto end_wall = std::chrono::high_resolution_clock::now();

    // auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_wall - start_wall).count();
    // uint64_t duration_cycles = end_cycles - start_cycles;

    // std::cout << "------------------------------------------------\n";

    // std::cout << "Processed:   " << order_book.get_processed_count() << " orders\n";
    // std::cout << "Wall Time:   " << duration_ms << " ms\n";
    // std::cout << "CPU Cycles:  " << duration_cycles << " (approx)\n";

    // if (duration_ms > 0) {
    //     double throughput = (double) order_book.get_processed_count() / duration_ms * 1000.0;
    //     std::cout << "Throughput:  " << (size_t) throughput << " orders/sec\n";
    //     std::cout << "Latency:     " << (double) duration_ms * 1000.0 / order_book.get_processed_count() << " us/order (amortized)\n";
    // }

    // std::cout << "------------------------------------------------\n";

    const std::string test_file = "01302020.NASDAQ_ITCH50";

    size_t file_size_bytes = 0;
    file_size_bytes = std::filesystem::file_size(test_file);

    std::cout << "Starting Engine...\n";
    
    uint64_t start_cycles = Utils::get_cycle_count();
    auto start_wall = std::chrono::high_resolution_clock::now();

    std::thread producer(feed_thread_func, test_file);
    std::thread consumer(engine_thread_func);

    producer.join();
    consumer.join();

    uint64_t end_cycles = Utils::get_cycle_count();
    auto end_wall = std::chrono::high_resolution_clock::now();

    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_wall - start_wall).count();

    uint64_t duration_cycles = end_cycles - start_cycles;
    uint64_t total_orders = order_book.get_processed_count();

    std::cout << "------------------------------------------------\n";
    std::cout << "RESUME STATISTICS GENERATOR\n";
    std::cout << "------------------------------------------------\n";

    if (duration_ns > 0) {
        double seconds = (double) duration_ns / 1'000'000'000.0;
        size_t throughput = (size_t) (total_orders / seconds);
        std::cout << "Throughput:       " << throughput << " orders/sec\n";
        
        double latency_ns = (double) duration_ns / total_orders;
        std::cout << "Average Latency:  " << latency_ns << " ns/order\n";

        if (file_size_bytes > 0) {
            double mb_per_sec = (double) (file_size_bytes / 1024.0 / 1024.0) / seconds;
            std::cout << "Data Bandwidth:   " << mb_per_sec << " MB/s\n";
        }
    }
    
    std::cout << "CPU Cycles:       " << duration_cycles << "\n";
    std::cout << "Cycles per Order: " << (duration_cycles / (total_orders > 0 ? total_orders : 1)) << "\n";

    std::cout << "------------------------------------------------\n";

    return 0;
}