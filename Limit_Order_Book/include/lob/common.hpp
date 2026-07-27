#pragma once
#include <cstdint>

inline constexpr uint64_t PRICE_SCALE = 10000;

namespace lob{
    class PriceQueue;
    enum class Side : uint8_t{
        Buy,
        Sell
    };
    
    using OrderId = uint64_t;
    using Price = uint64_t;
    using Total_Volume = uint32_t;
    using Remaining_volume = uint32_t;

    struct OrderNode{
        OrderId order_id;
        Price price;
        Total_Volume total_volume;
        Remaining_volume remaining_volume;
        OrderNode* next{nullptr};
        OrderNode* prev{nullptr};
        PriceQueue* parent_queue{nullptr};
        Side side;

        OrderNode() = default;
        OrderNode(uint64_t id, uint64_t p, uint32_t tv, uint32_t rv, Side s): order_id(id), price(p), total_volume(tv), remaining_volume(rv), side(s){}
    };
}