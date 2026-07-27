#pragma once
#include <mutex>

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
    SPSC(Callback callback):process(callback)
    {
        consumer_thread = std::thread([this]{
            while(running){
                Consume();
            }
        });   
    }

    void PushWork(const DataWrapper<T>& wrapper)
    {
        Node* new_node = new Node{wrapper, nullptr};
        std::unique_lock<std::mutex> lock(mut);
        if(!tail){
            head = tail = new_node;
        }
        else if(tail !=nullptr){
            tail -> next = new_node;
            tail = new_node;
        }
    }
    SPSC(const SPSC&) = delete;
    SPSC(SPSC&&) = delete;
    SPSC& operator =(const SPSC&) = delete;
    SPSC& operator =(SPSC&&) = delete;

    ~SPSC(){
        running = false;
        if(consumer_thread.joinable()){
            consumer_thread.join();
        }

        std::unique_lock<std::mutex> lock(mut);
        while(head){
            Node* temp = head;
            head = head -> next;
            delete temp;
        }
        tail = nullptr;
    } 

private:

    struct Node{
        DataWrapper<T> data;
        Node* next{nullptr};
    };
    Callback process;
    std::atomic<bool> running{true};
    std::mutex mut;

    Node* head{nullptr};
    Node* tail{nullptr};
    std::thread consumer_thread;

    void Consume()
    {
        Node* temp = nullptr;
        {
            std::lock_guard<std::mutex> lock(mut);
            if(!head) return;

            temp = head;
            head = head ->next ;

            if(!head){
                tail = nullptr;
            }
        }
        process(temp -> data.data);

        if(temp -> data.is_last_chunk){
            running = false;
        }
        delete temp;

    }
};