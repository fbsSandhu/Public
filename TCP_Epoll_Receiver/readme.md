## Architecture & Implementation

### Asynchronous Network Ingestion
* **POSIX Non-Blocking I/O:** Socket multiplexing via Linux `epoll` edge/level-triggered events.
* **`SO_REUSEADDR`:** Configured prior to `bind()` to eliminate port lockouts during rapid process restarts and bypass `TIME_WAIT` latency stalls.
* **`TCP_NODELAY`:** Disables Nagle's algorithm to force immediate packet flushing across the network.
* **Connection Lifecycle State Machine:** Tracks active file descriptors and clean peer disconnects (`recv() == 0`) for deterministic teardown.

### Lock-Free SPSC Queue
* **Zero-Allocation Storage:** Contiguous, pre-allocated ring buffer with capacity constrained to a power of two ($2^N$).
* **Bitwise Masking:** Replaces CPU modulo instructions (`%`) on index wrapping with bitwise AND logic 
* **Memory Model:** Employs explicit atomic synchronization (`std::memory_order_release` on writes, `std::memory_order_acquire` on reads) to eliminate mutexes and kernel context switches.
* **Cache Isolation:** Producer and consumer indices are isolated on separate 64-byte cache lines via `alignas(64)` to eliminate false sharing.

## Performance Benchmark

Stress-tested via automated payload generation over network loopback and physical bridge connections:

| Metric | Latency |
| :--- | :--- |
| **Min** | `67.6 µs` |
| **Avg** | `197.6 µs` |
| **Max (p99+ tail)** | `743.0 µs` |

## Build & Test Automation

The CMake configuration provides an automated pipeline that builds the binary, launches the receiver in the background, waits for socket binding, and triggers the test client.

```bash
# Configure
cmake -B build

# Build and execute end-to-end automated test suite
cmake --build build --target run_all
