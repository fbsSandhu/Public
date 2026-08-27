#include "tcp_receiver.hpp"

TCPReceiver::TCPReceiver(LFSPSCQ<Package, CAPACITY>* target_queue)
{
    queue = target_queue;
}

TCPReceiver::~TCPReceiver()
{
    flag.store(false, std::memory_order_release);
    if(thread_.joinable()){
        thread_.join();
    }
}

bool TCPReceiver::init(std::string hostname, int32_t portid, std::atomic<bool>* done_flag)
{
    host = hostname;
    port = portid;
    done = done_flag;

    listening_sock = socket(AF_INET, SOCK_STREAM, 0);

    if(listening_sock < 0){
        std::cout << "SOCKET CREATION ERROR" << std::endl;
        return false;
    }
    int opt = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "setsockopt failed!" << std::endl;
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    memset(server_addr.sin_zero, 0, 8);

    int success = bind(listening_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (success < 0){
        std::cout << "BIND FAIL" << std::endl;
        perror("bind");
        close(listening_sock);
        return false;
    }
    success = listen(listening_sock, 5);

    if (success < 0){
        std::cout << "LISTEN FAIL" << std::endl;
        return false;
    }
    
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listening_sock;

    epoll_fd = epoll_create(1);
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listening_sock, &event);

    return true;
}

void TCPReceiver::start()
{
    thread_ = std::thread(&TCPReceiver::run, this);
    pthread_t handle = thread_.native_handle();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
}

void TCPReceiver::run()
{   
    struct epoll_event events[10];
    while(flag){
        int n = epoll_wait(epoll_fd, events, 10, -1);
        if(n < 0) {
            std::cout << "WAIT ERROR" << std::endl;
            break;
        }    
        
        for (int i{}; i < n; ++i) {
            int ready_fd = events[i].data.fd;
            
            if(ready_fd == listening_sock) {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(listening_sock, (struct sockaddr*)&client_addr, &len);

                if(client_fd < 0) continue;
                
                struct epoll_event client_event;
                client_event.events = EPOLLIN;
                client_event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);

                ++active_clients;
                ++total_clients_served;
                
            }else{
                read_and_parse(ready_fd);
            }
        }

        if(total_clients_served > 0 && active_clients == 0){
            done -> store(true, std::memory_order_release);
            break;
        }
        
    }

    std::cout << "Producer Thread ended" << std::endl;
}

void TCPReceiver::read_and_parse(int client_sock)
{
    char temp[1024];
    ssize_t n = recv(client_sock, temp, 1024, 0);

    if(n <= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, NULL);
        close(client_sock);
        client_buffers.erase(client_sock);

        --active_clients;

        std::cout << "CLOSING CLIENT SOCK" << std::endl;
        return;
    }

    auto& buf = client_buffers[client_sock];
    buf.insert(buf.end(), temp, temp+n);
    while(buf.size() >= sizeof(Package)) {
        Package* data = (Package*)buf.data();
        data ->timestamp_ns = be64toh(data ->timestamp_ns);
        data -> order_id = be64toh(data -> order_id);
        data -> price = ntohl(data -> price);
        data -> quantity = ntohl(data -> quantity);

        if(!queue -> try_push(*data)) return;

        buf.erase(buf.begin(), buf.begin() + sizeof(Package));
    }
}

void TCPReceiver::stop()
{
    flag.store(false, std::memory_order_release);

    int dummy_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(dummy_sock < 0) return;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    memset(addr.sin_zero, 0, 8);

    connect(dummy_sock, (struct sockaddr*)&addr, sizeof(addr));
    close(dummy_sock);

    if(thread_.joinable()){
        thread_.join();
    }
    done -> store(true, std::memory_order_release);

}

