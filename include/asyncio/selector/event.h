//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_EVENT_H
#define ASYNCIO_EVENT_H

#include <asyncio/asyncio_ns.h>
#include <asyncio/handle.h>
#include <cstdint>

#if defined(__APPLE__)
    #include <sys/event.h>
    using Flags_t = int16_t;
    using Socket_t = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#elif defined(__linux__)
    #include <sys/epoll.h>
    using Flags_t = uint32_t;
    using Socket_t = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#elif defined(_WIN32)
    #include <WinSock2.h>
    using Flags_t = int16_t;
    using Socket_t = SOCKET;
#define close   closesocket
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#else
    #error "Support only Linux & MacOS!"
#endif

ASYNCIO_NS_BEGIN
struct Event {
    enum Flags: Flags_t {
    #if defined(__APPLE__)
        EVENT_READ = EVFILT_READ,
        EVENT_WRITE = EVFILT_WRITE
    #elif defined(__linux__)
        EVENT_READ = EPOLLIN,
        EVENT_WRITE = EPOLLOUT
    #elif defined(_WIN32)
        EVENT_READ = 1,
        EVENT_WRITE = 2,
    #else
        #error "Support only Linux & MacOS!"
    #endif
    };

    Socket_t fd;
    Flags flags;
    HandleInfo handle_info;
};
ASYNCIO_NS_END

#endif //ASYNCIO_EVENT_H
