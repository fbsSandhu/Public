#pragma once
#include "common.hpp"

namespace lob{
    class PriceQueue{
    private:
        lob::OrderNode* head{};
        lob::OrderNode* tail{};
        lob::Price price{};
        lob::Total_Volume total_volume{};
        int64_t order_count{};
    public:
        PriceQueue() = default;
        PriceQueue(lob::Price cprice) : price(cprice){}
        void append(lob::OrderNode* node){
            ++order_count;
            total_volume += node -> total_volume;
            node -> parent_queue = this;
            if(head == nullptr){
                head = node;
                tail = node;
            }else{
                tail->next = node;
                node -> prev = tail;
                tail = node;
            }
        }
        void remove(lob::OrderNode* node){
            --order_count;
            total_volume -= node -> remaining_volume; // remaining or total come back later
            if(head == tail){
                head = nullptr;
                tail = nullptr;
                node -> next = nullptr;
                node -> prev = nullptr;
            }else{
                if(node == head){
                    head = node -> next;
                    node -> next = nullptr;
                    head -> prev = nullptr;
                }else if(node == tail){
                    tail = node -> prev;
                    node -> prev = nullptr;
                    tail -> next = nullptr;
                }else{
                    node -> prev -> next = node -> next;
                    node -> next -> prev = node -> prev;
                    node -> next = nullptr;
                    node -> prev = nullptr;
                }
            }
        }
        void deduct_match_volume(uint32_t volume){
            total_volume -= volume;
        }

        lob::OrderNode* get_head() const {return head;}
        lob::Price get_price() const {return price;}
        lob::Total_Volume get_total_volume() const {return total_volume;}
        int64_t get_count() const {return order_count;} 

    };
}

