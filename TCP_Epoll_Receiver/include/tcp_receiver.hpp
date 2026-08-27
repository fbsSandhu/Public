#pragma once
#include "LFSPSCQ.hpp"
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <atomic>
#include <map>
#include <vector>
#include <iostream>

constexpr size_t CAPACITY{1024};

enum class Side : uint8_t {
    Buy,
    Sell
};

struct Package{
    uint64_t timestamp_ns;
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    Side side;
};

class TCPReceiver{
    LFSPSCQ<Package, CAPACITY>* queue;
    std::map<int, std::vector<char>> client_buffers;

    int32_t epoll_fd;
    int32_t listening_sock;
    struct epoll_event events[10];

    int32_t port;
    std::string host;

    int active_clients{};
    int total_clients_served{};

    std::atomic<bool> flag{true};
    std::atomic<bool>* done;
    std::thread thread_;

    void run();
    void read_and_parse(int client_sock);

public:
    TCPReceiver(LFSPSCQ<Package, CAPACITY>* target_queue);
    ~TCPReceiver();
    bool init(std::string hostname, int32_t portid, std::atomic<bool>* done_flag);
    void start();
    void stop();
    
    
};