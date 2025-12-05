# Hyperion

Hyperion is a high-performance, low-latency trading engine simulation written in C++20. It is designed to parse NASDAQ ITCH 5.0 market data feeds and process orders using lock-free concurrency patterns and memory-efficient data structures.

The project demonstrates techniques used in High-Frequency Trading (HFT), including memory pooling, lock-free ring buffers, thread pinning, and cache-line alignment.

## 🚀 Key Features

*   **Lock-Free Concurrency**: Implements a Single-Producer-Single-Consumer (SPSC) [`RingBuffer`](src/core/RingBuffer.hpp) using `std::atomic` with acquire/release memory ordering to facilitate zero-contention communication between the feed handler and the matching engine.
*   **Memory Management**: Utilizes a pre-allocated [`ObjectPool`](src/core/ObjectPool.hpp) to eliminate heap allocation overhead during the hot path execution.
*   **Zero-Copy Parsing**: Uses `mmap` in [`ITCHHandler`](src/feed/ITCHHandler.hpp) to map market data files directly into memory, avoiding unnecessary kernel-to-user space copies.
*   **Hardware Optimization**:
    *   **Cache Alignment**: Critical data structures like [`Order`](src/core/Types.hpp) are aligned to 64 bytes to prevent false sharing.
    *   **Thread Pinning**: Utilities in [`Thread.hpp`](src/utils/Thread.hpp) pin execution threads to specific CPU cores to maximize cache locality and reduce context switching.
    *   **Endianness Handling**: Efficient byte-swapping intrinsics in [`Endian.hpp`](src/utils/Endian.hpp) to convert Big-Endian network data to Little-Endian host format.
*   **Cross-Platform**: Supports both x86_64 (Linux) and ARM64 (Apple Silicon) architectures.

## 🏗 Architecture

The system operates on a pipelined architecture:

1.  **Feed Thread (Producer)**:
    *   Reads raw binary NASDAQ ITCH 5.0 data.
    *   Parses "Add Order" (Type 'A') messages.
    *   Converts network byte order to host byte order.
    *   Pushes normalized `Order` objects into the Ring Buffer.

2.  **Engine Thread (Consumer)**:
    *   Polls the Ring Buffer for new orders.
    *   Retrieves order objects from the `ObjectPool`.
    *   Passes orders to the [`OrderBook`](src/engine/OrderBook.hpp) for processing.

## 🛠 Build Instructions

### Prerequisites
*   **C++ Compiler**: GCC 10+ or Clang 12+ (Must support C++20).
*   **CMake**: Version 3.20 or higher.
*   **Make** or **Ninja**.

### Building the Project

1.  Create a build directory:
    ```sh
    mkdir build
    cd build
    ```

2.  Configure the project with CMake:
    ```sh
    cmake ..
    ```

3.  Compile the executable:
    ```sh
    make
    ```

## 🏃 Usage

Hyperion expects a NASDAQ ITCH 5.0 binary file. By default, [`main.cpp`](src/main.cpp) looks for a file named `01302020.NASDAQ_ITCH50` in the working directory.

1.  Ensure your ITCH data file is available.
2.  Run the executable:
    ```sh
    ./Hyperion
    ```

### Output Metrics
Upon completion, the engine outputs performance statistics:
*   **Throughput**: Orders processed per second.
*   **Average Latency**: Nanoseconds per order.
*   **Data Bandwidth**: MB/s processed from the feed.
*   **CPU Cycles**: Total CPU cycles consumed (using `rdtsc` on x86 or `cntvct_el0` on ARM).

## 📂 Project Structure

```text
src/
├── core/
│   ├── ObjectPool.hpp    # Fixed-size memory pool
│   ├── RingBuffer.hpp    # Lock-free SPSC queue
│   └── Types.hpp         # Common data structures (Order, Side)
├── engine/
│   └── OrderBook.hpp     # Order processing logic
├── feed/
│   └── ITCHHandler.hpp   # NASDAQ ITCH 5.0 parser
├── utils/
│   ├── Endian.hpp        # Byte swapping intrinsics
│   ├── Thread.hpp        # CPU affinity/pinning
│   └── Time.hpp          # Cycle counting
└── main.cpp              # Entry point and thread management
```

## 🔮 Future Improvements

To further enhance the engine's capabilities and performance, the following improvements are planned:

*   **Full ITCH Protocol Support**:
    *   Implement parsing for Order Executed, Order Cancel, Order Delete, and Order Replace messages to maintain an accurate state of the book.
    *   Handle System Event messages for market open/close logic.

*   **Matching Engine Logic**:
    *   Expand `OrderBook.hpp` to implement a true Limit Order Book (LOB) with Price/Time priority matching.
    *   Implement dual-sided book management (Bids and Asks).

*   **Network Integration**:
    *   Replace file replay with a UDP Multicast receiver to process live market data.
    *   Implement kernel bypass networking (e.g., DPDK or Solarflare OpenOnload) for lower network latency.

*   **SIMD Optimization**:
    *   Utilize AVX2/AVX-512 (x86) or NEON (ARM) intrinsics to parse multiple ITCH messages in parallel within the `ITCHHandler`.

*   **Logging & Analysis**:
    *   Implement a lock-free asynchronous logger to record latency spikes without blocking the hot path.
    *   Add histogram generation for latency distribution (p50, p99, p99.9).