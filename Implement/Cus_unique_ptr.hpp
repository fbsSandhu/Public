#pragma once
namespace impl
{
    template <typename T>
    struct custom_deleter
    {
        void operator()(T* pointer) const
        {
            delete pointer;
        }
    };

    template <typename T, typename custom_deleter = custom_deleter<T>>
    class unique_ptr
    {
    public:
        unique_ptr() 
        {   
            ptr = nullptr;
        }
        unique_ptr(T* pointer) : ptr(pointer)
        {
        }

        unique_ptr(const unique_ptr&){
            throw std::runtime_error("Cannot construct by copying another unique pointer");
        }
        unique_ptr& operator=(const unique_ptr&){
            throw std::runtime_error("Cannot copy a unique pointer");
            return *this;
        }

        unique_ptr(unique_ptr&& other) noexcept
        {
            if(this != &other){
                ptr = other.ptr;
                other.ptr = 0;
            }

        }

        unique_ptr& operator=(unique_ptr&& other) noexcept
        {
            deleter(ptr);
            if(this != &other){
                ptr = other.ptr;
                other.ptr = 0;
            }
            return *this;
        }

        ~unique_ptr()
        {
            custom_deleter deleter;
            deleter(ptr);
        }

        T* release()
        {
            T* temp = ptr;
            ptr = nullptr;
            return temp;
        }

        void reset(T* pointer)
        {
            if(ptr){
                custom_deleter deleter;
                deleter(ptr);
                ptr = nullptr;
            }
            if(pointer){
                ptr = pointer;
            }
        }

        bool is_owning() const { return ptr; }

        T& operator*() const { return *ptr;}
        T* operator->() const { return ptr;}
        explicit operator bool() const { return ptr;}

    private:
        T* ptr;

    };
}