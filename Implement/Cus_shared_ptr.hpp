#pragma once
#include <mutex>

namespace impl 
{
    struct ControlBlock
    {
        size_t count_{ 1 };
        mutable std::mutex mutex_;
    };

    template <typename T>
    class SharedPointer
    {
    public:

        SharedPointer(): ptr(nullptr), cb(nullptr) { }
        SharedPointer(T* pointer): ptr(pointer)
        {
            if(ptr){
                cb = new ControlBlock();
            }else{
                cb = nullptr;
            }
        }

        SharedPointer(const SharedPointer& other) noexcept: ptr(other.ptr), cb(other.cb) 
        {
            if(cb){
                std::lock_guard<std::mutex> lock(cb -> mutex_);
                ++cb -> count_;
            }
        }

        SharedPointer& operator=(const SharedPointer& other) noexcept
        {
            if(this == &other){
                return *this;
            }
            if(cb){
                bool should_delete{false};
                {
                    std::lock_guard<std::mutex> lock(cb ->mutex_);
                    --(cb -> count_);
                    if(cb -> count_ == 0)should_delete = true;
                }
                if(should_delete){
                    delete cb;
                    delete ptr;
                }
            }
            ptr = other.ptr;
            cb = other.cb;
            std::lock_guard<std::mutex> lock(cb -> mutex_);
            ++(cb -> count_);
            return *this;
        }

        SharedPointer(SharedPointer&& other)noexcept : ptr(other.ptr), cb(other.cb) 
        {
            other.ptr = nullptr;
            other.cb = nullptr;
        }

        SharedPointer& operator=(SharedPointer&& other) noexcept
        {
            if(this == &other){
                return *this;
            }

            if(cb){
                bool should_delete{false};
                {
                    std::lock_guard<std::mutex> lock(cb ->mutex_);
                    --(cb -> count_);
                    if(cb -> count_ == 0)should_delete = true;
                }
                if(should_delete){
                    delete cb;
                    delete ptr;
                }
            }
            ptr = other.ptr;
            cb = other.cb;
            other.cb = nullptr;
            other.ptr = nullptr;
            return *this;
        }

        ~SharedPointer()
        {
            if(ptr){
                bool should_delete{false};
                {
                    std::lock_guard<std::mutex> lock(cb -> mutex_);
                    --(cb -> count_);
                    if(cb -> count_ == 0) should_delete = true;
                }
                if(should_delete){
                    delete cb;
                    delete ptr;
                }
            }
        }

        void reset(T* pointer)
        {
            if(cb){
                bool should_delete{false};
                {
                    std::lock_guard<std::mutex> lock(cb -> mutex_);
                    --(cb -> count_);
                    if(cb -> count_ == 0) should_delete = true;
                }
                if(should_delete){
                    delete cb;
                    delete ptr;
                }
            }
            
            if(pointer){
                ptr = pointer;
                cb = new ControlBlock();
            }else{
                ptr = nullptr;
                cb = nullptr;
            }
        }


        size_t get_count() const
        {
            if(!cb){
                return 0;
            }
            std::lock_guard<std::mutex> lock(cb -> mutex_);
            return cb -> count_;
        }

        T* operator->() const { 
            return ptr;
        }
        T& operator*() const { 
            if(ptr){
                return *ptr;
            }else{
                throw std::runtime_error("Attempted derference of nullptr");
            }
        }
        operator bool() const noexcept { 
            return ptr != nullptr;
        }

    private:
    T* ptr{nullptr};
    ControlBlock* cb{nullptr};
    };
}