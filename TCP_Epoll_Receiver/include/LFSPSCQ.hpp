#pragma once
#include <atomic>
#include <cstddef>
#include <array>
#include <new>
#include <utility>

template<typename T, size_t capacity>
class LFSPSCQ{
    static_assert((capacity & (capacity - 1)) == 0, "Capacity must be power of 2");
public:
    LFSPSCQ(): head(0), tail(0) {}
    
    template<typename... Args>
    bool try_push(Args&&... args) {
        const auto curr_tail = tail.load(std::memory_order_relaxed);
        const auto curr_head = head.load(std::memory_order_acquire);
        if(curr_tail - curr_head >= capacity) return false;

        buffer[curr_tail & (capacity - 1)] = T(std::forward<Args>(args)...);
        

        tail.store(curr_tail + 1, std::memory_order_release);
        return true;
        
    }
    bool try_pop(T& value) {
        const auto curr_head = head.load(std::memory_order_relaxed);
        const auto curr_tail = tail.load(std::memory_order_acquire);

        if(curr_tail == curr_head) return false;

        value = std::move(buffer[curr_head & (capacity - 1)]);
        head.store(curr_head + 1, std::memory_order_release);
        return true;
    }
    
    ~LFSPSCQ(){}
    
private:
    alignas(64) std::array<T, capacity> buffer;
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
};