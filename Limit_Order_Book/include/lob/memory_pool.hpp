#pragma once 
#include <utility>
#include <array>
#include <new>
#include <cstddef>
#include <cstdint>
#include <stdexcept>


template<typename T, size_t Capacity>
class MemoryPool {
private:
    alignas(T) std::array<std::byte, sizeof(T) * Capacity> storage;
    std::array<uint32_t, Capacity> freeStack;
    uint32_t index_track{Capacity};

public:
    MemoryPool() {
        for (uint32_t i = 0; i < Capacity; ++i) {
            freeStack[i] = i;
        }
    }

    ~MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    template<typename... Args>
    T* acquire(Args&&... args) {
        if (index_track == 0) [[unlikely]] {
            throw std::runtime_error("MemoryPool exhausted!");
        }

        --index_track;
        uint32_t slot = freeStack[index_track];
        
        T* ptr = reinterpret_cast<T*>(&storage[slot * sizeof(T)]);
        return ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
    }

    void release(T* ptr) noexcept {
        if (ptr == nullptr) [[unlikely]] return;

        ptr->~T();
        freeStack[index_track] = static_cast<uint32_t>(
            (reinterpret_cast<uint8_t*>(ptr) - reinterpret_cast<uint8_t*>(storage.data())) / sizeof(T)
        );
        ++index_track;
    }
};