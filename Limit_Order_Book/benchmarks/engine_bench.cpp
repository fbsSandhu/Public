#include <benchmark/benchmark.h>
#include <random>
#include <memory> 
#include <lob/engine.hpp>

static void BM_PassiveOrderInsertion(benchmark::State& state){
    auto engine = std::make_unique<Engine>();
    uint64_t order_id{1};
    uint64_t price{100000};

    for(auto _ : state){
        bool success = engine -> submit_order(order_id++, lob::Side::Buy, price, 10);
        benchmark::DoNotOptimize(success);
        benchmark::ClobberMemory();

        if(order_id >= 800'000) [[unlikely]]{
            state.PauseTiming();
            engine = std::make_unique<Engine>();
            order_id = 1;
            state.ResumeTiming();
        }
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_PassiveOrderInsertion);

static void BM_OrderCancellation(benchmark::State& state) {
    constexpr uint64_t num_orders{500'000};
    auto engine = std::make_unique<Engine>();
    uint64_t order_id{1};
    for(uint64_t i{1}; i <= num_orders; ++i){
        engine -> submit_order(i, lob::Side::Buy, 100000, 10);
    }
    for(auto _ : state){
        bool success = engine -> cancel_order(order_id++);
        benchmark::DoNotOptimize(success);

        if(order_id >= num_orders) [[unlikely]]{
            state.PauseTiming();
            engine = std::make_unique<Engine>();
            for(uint64_t i{1}; i <= num_orders; ++i){
                engine -> submit_order(i, lob::Side::Buy, 100000, 10);
            }
            order_id = 1;
            state.ResumeTiming();
        }
    }
    state.SetItemsProcessed(state.iterations());

}

BENCHMARK(BM_OrderCancellation);

static void BM_ContinuosMatching(benchmark::State& state){
    auto engine = std::make_unique<Engine>();
    uint64_t order_id{1};
    uint64_t price{100000};

    for(auto _ : state){
        engine -> submit_order(order_id++, lob::Side::Buy, price, 10);
        bool success = engine -> submit_order(order_id++, lob::Side::Sell, price, 10);
        benchmark::DoNotOptimize(success);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * 2);
    
}
BENCHMARK(BM_ContinuosMatching);

inline uint32_t xorshift(uint32_t& state){
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
}

static void BM_RandomPriceInsertion(benchmark::State& state){
    auto engine = std::make_unique<Engine>();
    uint64_t order_id{1};
    uint32_t rng_state{123456789};

    for(auto _: state){
        uint64_t price = 99000 + (xorshift(rng_state) % 2000);

        bool success = engine -> submit_order(order_id++, lob::Side::Buy, price, 10);
        benchmark::DoNotOptimize(success);
        if(order_id >= 800'000) [[unlikely]]{
            state.PauseTiming();
            engine = std::make_unique<Engine>();
            order_id = 1;
            state.ResumeTiming();
        }
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_RandomPriceInsertion);
BENCHMARK_MAIN();