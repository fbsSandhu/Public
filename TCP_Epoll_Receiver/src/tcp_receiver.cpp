#include "tcp_receiver.hpp"

TCPReceiver::TCPReceiver(LFSPSCQ<Package, CAPACITY>* target_queue)
{
    queue = target_queue;
    client_buffers.resize(64);
}

TCPReceiver::~TCPReceiver()
{
    flag.store(false, std::memory_order_release);
    if(thread_.joinable()){
        thread_.join();
    }

    if(epoll_fd >= 0) close(epoll_fd);
    if(listening_sock >= 0) close(listening_sock);
}

bool TCPReceiver::init(std::string hostname, int32_t portid, std::atomic<bool>* done_flag)
{
    host = hostname;
    port = portid;
    done = done_flag;

    listening_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if(listening_sock < 0){
        std::cerr << "SOCKET CREATION ERROR\n";
        return false;
    }
    int opt = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed!\n";
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    memset(server_addr.sin_zero, 0, 8);

    if(bind(listening_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        close(listening_sock);
        return false;
    }


    if(listen(listening_sock, 128) < 0 ){
        perror("listen");
        close(listening_sock);
        return false;
    }

    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0 ){
        perror("epoll_create1");
        close(listening_sock);
        return false;
    }
    
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listening_sock;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listening_sock, &event) < 0){
        perror("Epoll_ctl and listening sock");
        close(listening_sock);
        close(epoll_fd);
        return false;
    }

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
    struct epoll_event events[16];
    while(flag.load(std::memory_order_relaxed)){
        int n = epoll_wait(epoll_fd, events, 16, -1);
        if(n < 0) {
            if(errno == EINTR) continue;
            std::cerr << "WAIT ERROR" << std::endl;
            break;
        }    
        
        for (int i{}; i < n; ++i) {
            int ready_fd = events[i].data.fd;
            
            if(ready_fd == listening_sock) {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);


                int client_fd = accept4(listening_sock, (struct sockaddr*)&client_addr, &len, SOCK_NONBLOCK);

                if(client_fd < 0){
                    if(errno == EAGAIN || errno == EWOULDBLOCK){
                        continue;
                    }
                    perror("accept4");
                    continue;
                }

                int nodelay = 1;
                setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
                
                if(client_fd >= static_cast<int>(client_buffers.size())){
                    client_buffers.resize(client_fd + 16);
                }
                client_buffers[client_fd].reset();

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
            if (done) done -> store(true, std::memory_order_release);
            break;
        }
        
    }

    std::cout << "Producer Thread ended" << std::endl;
}

void TCPReceiver::read_and_parse(int client_sock)
{
    char temp[1024];
    ssize_t n = recv(client_sock, temp, 1024, 0);

    if(n < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            return;
        }
        if(errno == EINTR){
            return;
        }
        perror("recv");
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, nullptr);
        close(client_sock);
        client_buffers[client_sock].reset();
        --active_clients;
        return;
    }
    if(n == 0){
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, nullptr);
        close(client_sock);
        client_buffers[client_sock].reset();;
        --active_clients;
        return;
    }

    auto& cb = client_buffers[client_sock];
    cb.buf.insert(cb.buf.end(), temp, temp + n);

    while(cb.unread_bytes() >= sizeof(Package)){
        Package pkg;
        std::memcpy(&pkg, cb.buf.data() + cb.read_idx, sizeof(Package));
        pkg.timestamp_ns = be64toh(pkg.timestamp_ns);
        pkg.order_id = be64toh(pkg.order_id);
        pkg.price = ntohl(pkg.price);
        pkg.quantity = ntohl(pkg.quantity);
        
        int retries{1000};

        while(!queue -> try_push(pkg) && retries-- > 0){
            #if defined(__x86_64__) || defined(_M_X64)
            __builtin_ia32_pause();
            #endif
        }
        
        if(retries <= 0) {
            break;
        }

        cb.read_idx += sizeof(Package);

    }

    cb.compact();
}

void TCPReceiver::stop()
{
    flag.store(false, std::memory_order_release);

    int dummy_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(dummy_sock >= 0) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);
        connect(dummy_sock, (struct sockaddr*)&addr, sizeof(addr));
        close(dummy_sock);
    }


    if(thread_.joinable()){
        thread_.join();
    }
    if (done) done -> store(true, std::memory_order_release);

}

