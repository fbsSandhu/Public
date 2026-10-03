#include <tcp_receiver.hpp>

constexpr size_t MAX_SAMPLES{1000000};

struct LatencySample{
    uint64_t send_time_ns;
    uint64_t receive_time_ns;
};


int main() {
    LFSPSCQ<Package, CAPACITY> lfqueue;
    TCPReceiver receiver(&lfqueue);

    std::vector<LatencySample> latencies(MAX_SAMPLES);
    std::atomic<size_t> sample_count{};
    std::atomic<bool> producer_done{false};

    if(receiver.init("0.0.0.0", 8080, &producer_done)){
        receiver.start();
    }

    std::atomic<bool> consumer_running{true};
    
    std::thread consumer([&consumer_running, &lfqueue, &sample_count, &latencies]{
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(2, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

        while(consumer_running) {
            Package pck;
            if(lfqueue.try_pop(pck)){
                uint64_t now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                
                if(sample_count < MAX_SAMPLES){
                    latencies[sample_count].send_time_ns = pck.timestamp_ns;
                    latencies[sample_count].receive_time_ns = now;
                    ++sample_count;
                }
            }
        }
    });

    while(!producer_done.load(std::memory_order_acquire));



    consumer_running.store(false, std::memory_order_release); 
    if(consumer.joinable()) consumer.join();

    std::cout << "STATS" << std::endl;

    uint64_t min_latency{UINT64_MAX};
    uint64_t max_latency{}, sum{};

    for (size_t i{}; i < sample_count; ++i) {
        uint64_t lat = latencies[i].receive_time_ns - latencies[i].send_time_ns;
        max_latency = std::max(max_latency, lat);
        min_latency = std::min(min_latency, lat);
        sum += lat;
    }



    std::cout << "Min: " << min_latency << "ns" << std::endl;
    std::cout << "Max: " << max_latency << "ns" << std::endl;
    std::cout << "Avg: " << sum/sample_count << "ns" << std::endl;


    return 0;
}