# Low-Latency O(1) Limit Order Book Engine
A deterministic, single-threaded matching engine written in C++20. Designed for zero dynamic memory allocations on the hot execution path and deterministic constant-time $O(1)$ order operations

---
## Architecture & Low-Latency Design

* **Zero Allocation Hot-Path:** Pre-allocates order slots using custom memory pools to eliminate dynamic heap allocations (`new`/`delete`) during live order processing.
* **Intrusive Doubly-Linked Lists:** Nodes store pointers internally backed by a contiguous memory pool, maximizing CPU L1/L2 cache locality when traversing price levels.
* **O(1) Direct Lookup:** Uses `std::unordered_map` mapping Order IDs directly to intrusive list nodes for $O(1)$ cancellations and modifications with a 3 level bitmap for next best bid/ask price.

---

## Performance & Google Benchmark Results

### Test Environment
* **OS:** Windows 11 x86_64
* **Compiler:** GCC 11.4.0 (`-Wall -Wextra -Wpedantic -Werror -O3`)
* **Framework:** Google Benchmark v1.8.0

### Benchmark Execution Output
```text
-----------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------
BM_PassiveOrderInsertion       23.1 ns         22.5 ns     32000000 items_per_second=44.5217M/s
BM_OrderCancellation           15.9 ns         18.0 ns     65163637 items_per_second=55.6063M/s
BM_ContinuosMatching           56.6 ns         54.7 ns     10000000 items_per_second=36.5714M/s
BM_RandomPriceInsertion        30.4 ns         30.0 ns     21333333 items_per_second=33.3008M/s
```
## Building & Running

### Prerequisites
* **C++ Compiler:** MSVC (Visual Studio 2022+), GCC 11+, or Clang 13+ with **C++20** support.
* **Build System:** [CMake](https://cmake.org/) (v3.18 or higher).
* **Dependencies:** Google Benchmark & Google Test *(automatically fetched via CMake `FetchContent`)*.

---

### Build Instructions

From the `Limit_Order_Book` directory, run:

```bash
#1. cmake --build build --target MatchingEngine
#2. cmake --build build --target engine_benchmark
```
And to build:
1. .\build\MatchingEngine.exe
2. .\build\benchmarks\engine_benchmark.exe


