#pragma once
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

template <typename T>
struct DataWrapper
{
    T data{ };
    bool is_last_chunk{ false };
};

template <typename T, typename Callback>
class SPSC
{
public:
    SPSC(Callback callback):process(callback): process(callback), consumer_{[this]{Consume();}}
    { }
    void PushWork(const DataWrapper<T>& wrapper){
        {
            std::scoped_lock lock{mut};
            queue_.push(wrapper);
        }
        cv.notify_one();
    }
    ~SPSC()
    {
        flag.store(true, std::memory_order_relaxed);
        PushWork({}, true);
        consumer_.join();
    }


private:
    void consumer(){
        while(!flag.load(std::memory_order_relaxed)){
            std::unique_lock lock{mut};
            if(queue_.empty()){
                cv.wait(lock, [this]{return !queue_.empty();});
            }
            if(flag.load(std::memory_order_relaxed)) break;
            auto consumable = queue_.front();
            queue_.pop();
            lock.unlock();
            process(consumable.data);
            if(consumable.is_last_chunk) break;
        }

    }
    Callback process;
    mutable std::mutex mut{};
    std::queue<DataWrapper<T>> queue_{};
    std::thread consumer_{};
    std::condition_variable cv{};
    std::atomic<bool> flag{false};
};