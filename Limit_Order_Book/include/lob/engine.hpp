#pragma once
#include "memory_pool.hpp"
#include "price_queue.hpp"
#include <unordered_map>
#include <memory_resource>
#include <bit>
#include <memory>




    
class Engine{
    private:
    static constexpr uint64_t MAXPRICETICKS = 500'000;
    static constexpr uint64_t L2_SIZE = (MAXPRICETICKS + 63) / 64;
    static constexpr uint64_t L1_SIZE = (L2_SIZE + 63) / 64;
    static constexpr uint64_t L0_SIZE = (L1_SIZE + 63) / 64;
    std::array<uint64_t, L2_SIZE> buy_l2{};
    std::array<uint64_t, L1_SIZE> buy_l1{};
    std::array<uint64_t, L0_SIZE> buy_l0{};

    std::array<uint64_t, L2_SIZE> sell_l2{};
    std::array<uint64_t, L1_SIZE> sell_l1{};
    std::array<uint64_t, L0_SIZE> sell_l0{};


    std::unique_ptr<char[]> memory_buffer = std::make_unique<char[]>(1024*1024*16);
    std::pmr::monotonic_buffer_resource pool_resource{memory_buffer.get(), 1024*1024*16};
    std::pmr::unordered_map<lob::OrderId, lob::OrderNode*> order_map{&pool_resource};
    std::pmr::unordered_map<lob::Price, lob::PriceQueue*> buy_map{&pool_resource};
    std::pmr::unordered_map<lob::Price, lob::PriceQueue*> sell_map{&pool_resource};

    MemoryPool<lob::OrderNode, 1'000'000> Node_memory;
    MemoryPool<lob::PriceQueue, 100'000> Queue_memory;
    void activate_price_tick(lob::Side side, uint64_t price);
    void deactivate_price_tick(lob::Side side, uint64_t price);
    
    public:
    Engine() = default;
    ~Engine() = default;
    bool submit_order(uint64_t id, lob::Side side, uint64_t price, uint32_t volume);
    bool cancel_order(uint64_t id);
    uint64_t get_best_sell();
    uint64_t get_best_buy();
};
