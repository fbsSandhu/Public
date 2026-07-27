#include <lob/engine.hpp>

void Engine::activate_price_tick(lob::Side side, uint64_t price){
    uint64_t l2_idx = price / 64;
    uint64_t l2_bit = price % 64;

    uint64_t l1_idx = l2_idx / 64;
    uint64_t l1_bit = l2_idx % 64;
    
    uint64_t l0_idx = l1_idx / 64;
    uint64_t l0_bit = l1_idx % 64;

    if(side == lob::Side::Buy){
        buy_l2[l2_idx] |= (1ULL << l2_bit);
        buy_l1[l1_idx] |= (1ULL << l1_bit);
        buy_l0[l0_idx] |= (1ULL << l0_bit);
    }else{
        sell_l2[l2_idx] |= (1ULL << l2_bit);
        sell_l1[l1_idx] |= (1ULL << l1_bit);
        sell_l0[l0_idx] |= (1ULL << l0_bit);
    }
}

void Engine::deactivate_price_tick(lob::Side side, uint64_t price){
    uint64_t l2_idx = price / 64;
    uint64_t l2_bit = price % 64;

    uint64_t l1_idx = l2_idx / 64;
    uint64_t l1_bit = l2_idx % 64;
    
    uint64_t l0_idx = l1_idx / 64;
    uint64_t l0_bit = l1_idx % 64;

    if(side == lob::Side::Buy){
        buy_l2[l2_idx] &= ~(1ULL << l2_bit);
        if(buy_l2[l2_idx] == 0ULL){
            buy_l1[l1_idx] &= ~(1ULL << l1_bit);

            if(buy_l1[l1_idx] == 0ULL){
                buy_l0[l0_idx] &= ~(1ULL << l0_bit);
            }
        }
    }else{
        sell_l2[l2_idx] &= ~(1ULL << l2_bit);
        if(sell_l2[l2_idx] == 0ULL){
            sell_l1[l1_idx] &= ~(1ULL << l1_bit);

            if(sell_l1[l1_idx] == 0ULL){
                sell_l0[l0_idx] &= ~(1ULL << l0_bit);
            }
        }
    }
}

uint64_t Engine::get_best_sell(){
    for(size_t i = 0; i < L0_SIZE; ++i){
        if(sell_l0[i] != 0ULL){
            uint64_t l0_bit = std::countr_zero(sell_l0[i]);
            uint64_t l1_idx = (i * 64) + l0_bit;

            uint64_t l1_val = sell_l1[l1_idx];
            uint64_t l1_bit = std::countr_zero(l1_val);
            uint64_t l2_idx = (l1_idx * 64) + l1_bit;

            uint64_t l2_val = sell_l2[l2_idx];
            uint64_t l2_bit = std::countr_zero(l2_val);

            return (l2_idx * 64) + l2_bit;
        }
    }
    return 0;
}

uint64_t Engine::get_best_buy(){
    for(int64_t i = static_cast<int64_t>(L0_SIZE) - 1; i >=0; --i){
        if(buy_l0[i] != 0ULL){
            uint64_t l0_bit = 63 - std::countl_zero(buy_l0[i]);
            uint64_t l1_idx = (i*64) + l0_bit;

            uint64_t l1_word = buy_l1[l1_idx];
            uint64_t l1_bit = 63 - std::countl_zero(l1_word);
            uint64_t l2_idx = (l1_idx * 64) + l1_bit;

            uint64_t l2_word = buy_l2[l2_idx];
            uint64_t l2_bit = 63 - std::countl_zero(l2_word);
            return (l2_idx * 64 ) + l2_bit;
        }
    }
    return 0;
}

//submit order relies on the fact a valid volume is entered. if equal to zero will cause seg fault
bool Engine::submit_order(uint64_t id, lob::Side side, uint64_t price, uint32_t volume){
    lob::PriceQueue* target_queue = nullptr;
    uint64_t best_price;
    while(volume > 0){
        if(side == lob::Side::Sell){
            best_price = get_best_buy();
            if(best_price == 0 || price > best_price) break;
            auto it = buy_map.find(best_price);
            if(it == buy_map.end()) break;
            target_queue = it -> second;
        }else{
            best_price = get_best_sell();
            if(best_price == 0 || price < best_price) break;
            auto it = sell_map.find(best_price);
            if(it == sell_map.end()) break;
            target_queue = it -> second;
        }
        while(target_queue != nullptr && volume > 0 && target_queue -> get_count() != 0){
            lob::OrderNode* curr_head = target_queue -> get_head();
    
            uint32_t matching_vol = std::min(volume, curr_head -> remaining_volume);
            volume -= matching_vol;
            curr_head -> remaining_volume -= matching_vol;
            target_queue->deduct_match_volume(matching_vol);
    
            if(curr_head -> remaining_volume == 0){
                target_queue->remove(curr_head);
                order_map.erase(curr_head -> order_id);
                Node_memory.release(curr_head);
            }
        }
        if(target_queue && target_queue -> get_count() == 0){
            if(side == lob::Side::Buy){
                sell_map.erase(best_price);
                deactivate_price_tick(lob::Side::Sell, best_price);
            }
            else{
                buy_map.erase(best_price);
                deactivate_price_tick(lob::Side::Buy, best_price);
            }
            Queue_memory.release(target_queue);
        }
    }
    if(!volume){
        return true;
    }
    //now if remaining vol it needs to go into its respective queue
    if(side == lob::Side::Sell){
        auto it = sell_map.find(price);
        if(it != sell_map.end()){
            target_queue = it -> second;
        }else{
            target_queue = Queue_memory.acquire(price);
            activate_price_tick(side, price);
            sell_map[price] = target_queue;
        }
    }else{
        auto it = buy_map.find(price);
        if(it != buy_map.end()){
            target_queue = it -> second;
        }else{
            target_queue = Queue_memory.acquire(price);
            activate_price_tick(side, price);
            buy_map[price] = target_queue;
        }
    }

    lob::OrderNode* node = Node_memory.acquire(id, price, volume, volume, side);
    node -> parent_queue = target_queue;
    order_map[id] = node;
    target_queue->append(node);
    return true;
}

bool Engine::cancel_order(uint64_t id){
    auto it = order_map.find(id);
    if(it == order_map.end()) return false;

    lob::OrderNode* delNode = it ->second;
    lob::PriceQueue* target_queue = delNode -> parent_queue;
    target_queue -> remove(delNode);
    if(target_queue -> get_count() == 0){
        deactivate_price_tick(delNode ->side, delNode ->price);
        if(delNode -> side == lob::Side::Buy){
            buy_map.erase(delNode -> price);
        }else{
            sell_map.erase(delNode -> price);
        }
        Queue_memory.release(target_queue);
    }
    order_map.erase(it);
    Node_memory.release(delNode);
    return true;
}