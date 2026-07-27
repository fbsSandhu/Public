#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cstdint>
#include <lob/engine.hpp>

int main() {
    constexpr uint64_t TOTAL_OPERATIONS = 1000000;
    constexpr uint64_t BASE_PRICE = 100000;
    constexpr uint64_t PRICE_SPREAD = 50;

    auto engine = std::make_unique<Engine>();

    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<uint64_t> price_dist(BASE_PRICE - PRICE_SPREAD, BASE_PRICE + PRICE_SPREAD);
    std::uniform_int_distribution<uint32_t> std_qty_dist(1, 50);
    std::uniform_int_distribution<uint32_t> whale_qty_dist(500, 3000);
    std::uniform_int_distribution<uint32_t> op_dist(0, 99);

    std::vector<uint64_t> active_ids;
    active_ids.reserve(TOTAL_OPERATIONS);

    uint64_t total_matches = 0;
    uint64_t total_cancels = 0;
    uint64_t total_submissions = 0;

    std::cout << "Starting stress test: " << TOTAL_OPERATIONS << " operations..." << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 1; i <= TOTAL_OPERATIONS; ++i) {
        uint32_t op_type = op_dist(rng);

        if (op_type < 10 && !active_ids.empty()) {
            size_t idx = rng() % active_ids.size();
            uint64_t cancel_id = active_ids[idx];

            if (engine->cancel_order(cancel_id)) {
                total_cancels++;
            }

            active_ids[idx] = active_ids.back();
            active_ids.pop_back();
        } 
        else {
            lob::Side side = (rng() % 2 == 0) ? lob::Side::Buy : lob::Side::Sell;
            uint64_t price = price_dist(rng);
            uint32_t qty = (i % 500 == 0) ? whale_qty_dist(rng) : std_qty_dist(rng);

            bool matched = engine->submit_order(i, side, price, qty);

            if (matched) {
                total_matches++;
            }

            active_ids.push_back(i);
            total_submissions++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> elapsed_ns = end_time - start_time;

    double total_seconds = elapsed_ns.count() / 1e9;
    double ops_per_sec = TOTAL_OPERATIONS / total_seconds;
    double mops_per_sec = ops_per_sec / 1e6;
    double avg_latency_ns = elapsed_ns.count() / TOTAL_OPERATIONS;

    std::cout << "\n==================================================" << std::endl;
    std::cout << "         ENGINE STRESS TEST RESULTS               " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::left << std::setw(28) << "Total Operations:" << TOTAL_OPERATIONS << std::endl;
    std::cout << std::left << std::setw(28) << "Orders Submitted:" << total_submissions << std::endl;
    std::cout << std::left << std::setw(28) << "Matches Executed:" << total_matches << std::endl;
    std::cout << std::left << std::setw(28) << "Orders Canceled:" << total_cancels << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(28) << "Elapsed Time:" << std::fixed << std::setprecision(4) << total_seconds << " s" << std::endl;
    std::cout << std::left << std::setw(28) << "Throughput:" << std::fixed << std::setprecision(2) << ops_per_sec << " ops/sec" << std::endl;
    std::cout << std::left << std::setw(28) << "Throughput (MOps):" << std::fixed << std::setprecision(3) << mops_per_sec << " MOps/sec" << std::endl;
    std::cout << std::left << std::setw(28) << "Average Latency:" << std::fixed << std::setprecision(2) << avg_latency_ns << " ns/op" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}