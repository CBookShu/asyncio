//
// Created by netcan on 2021/10/10.
//
#include <asyncio/event_loop.h>
#include <asyncio/selector/selector.h>

#include "../test_tool.h"

using namespace ASYNCIO_NS;
using namespace std::chrono;

static void test_selector_wait() {
    EventLoop loop;
    Selector selector;
    auto before_wait = loop.time();
    selector.select(300);
    auto after_wait = loop.time();
    REQUIRE(after_wait - before_wait >= 300ms);
}

int main() {
    TEST_CALL(test_selector_wait);
    return 0;
}