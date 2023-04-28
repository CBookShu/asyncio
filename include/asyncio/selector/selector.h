//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_SELECTOR_H
#define ASYNCIO_SELECTOR_H

#if defined(__APPLE__)
#include <asyncio/selector/kqueue_selector.h>
namespace ASYNCIO_NS {
using Selector = KQueueSelector;
}
#elif defined(__linux)
#include <asyncio/selector/epoll_selector.h>
namespace ASYNCIO_NS {
using Selector = EpollSelector;
}
#elif defined(_WIN32)
#include <asyncio/selector/win32_selector.h>
namespace ASYNCIO_NS {
using Selector = Win32Selector;
}
#endif

#endif //ASYNCIO_SELECTOR_H
