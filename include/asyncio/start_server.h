//
// Created by netcan on 2021/11/30.
//

#ifndef ASYNCIO_START_SERVER_H
#define ASYNCIO_START_SERVER_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/stream.h>
#include <asyncio/finally.h>
#include <asyncio/schedule_task.h>
#include <list>
#include <sys/types.h>

ASYNCIO_NS_BEGIN
namespace concepts {
template<typename CONNECT_CB>
concept ConnectCb = requires(CONNECT_CB cb) {
    { cb(std::declval<Stream>()) } -> concepts::Awaitable;
};
}

constexpr static size_t max_connect_count = 16;

template<concepts::ConnectCb CONNECT_CB>
struct Server: NonCopyable {
    Server(CONNECT_CB cb, Socket_t fd): connect_cb_(cb), fd_(fd) {}
    Server(Server&& other): connect_cb_(other.connect_cb_),
                            fd_{std::exchange(other.fd_, -1) } {}
    ~Server() { close(); }

    Task<void> serve_forever() {
        Event ev { .fd = fd_, .flags = Event::Flags::EVENT_READ };
        auto& loop = get_event_loop();
        auto ev_awaiter = loop.wait_event(ev);
        std::list<ScheduledTask<Task<>>> connected;
        while (true) {
            sockaddr_storage remoteaddr{};
            socklen_t addrlen = sizeof(remoteaddr);
            co_await ev_awaiter;
            Socket_t clientfd = ::accept(fd_, reinterpret_cast<sockaddr*>(&remoteaddr), &addrlen);
            if (clientfd == -1) { continue; }
            connected.emplace_back(schedule_task(connect_cb_(Stream{clientfd, remoteaddr})));
            // garbage collect
            clean_up_connected(connected);
        }
    }

private:
    void clean_up_connected(std::list<ScheduledTask<Task<>>>& connected) {
        if (connected.size() < 100) [[likely]] { return; }
        for (auto iter = connected.begin(); iter != connected.end(); ) {
            if (iter->done()) {
                iter->get_result(); //< consume result, such as throw exception
                iter = connected.erase(iter);
            } else {
                ++iter;
            }
        }
    }

private:
    void close() {
        if (fd_ != INVALID_SOCKET) { ::close(fd_); }
        fd_ = INVALID_SOCKET;
    }

private:
    [[no_unique_address]] CONNECT_CB connect_cb_;
    Socket_t fd_{INVALID_SOCKET};
};

template<concepts::ConnectCb CONNECT_CB>
Task<Server<CONNECT_CB>> start_server(CONNECT_CB cb, std::string_view ip, uint16_t port) {
    addrinfo hints { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
    addrinfo *server_info {nullptr};
    auto service = std::to_string(port);
    // TODO: getaddrinfo is a blocking api
    if (int rv = getaddrinfo(ip.data(), service.c_str(), &hints, &server_info);
            rv != 0) {
        throw std::system_error(std::make_error_code(std::errc::address_not_available));
    }
    finally{ freeaddrinfo(server_info); };

    Socket_t serverfd = -1;
    for (auto p = server_info; p != nullptr; p = p->ai_next) {
        if ((serverfd = ::socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol)) == -1) {
            continue;
        }
        socket::set_blocking(serverfd, false);
        char yes = 1;
        // lose the pesky "address already in use" error message
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if ( bind(serverfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(serverfd);
        serverfd = -1;
    }
    if (serverfd == -1) {
        throw std::system_error(std::make_error_code(std::errc::address_not_available));
    }

    if (listen(serverfd, max_connect_count) == -1 && checkerror()) {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
    }

    co_return Server{cb, serverfd};
}

ASYNCIO_NS_END

#endif // ASYNCIO_START_SERVER_H
