//
// Created by netcan on 2021/11/20.
//

#include <asyncio/task.h>
#include <asyncio/runner.h>
#include <ranges>
#include <cassert>
#include "../test_tool.h"

using asyncio::Task;

static void lots_of_synchronous_completions() {
    auto completes_synchronously = []() -> Task<int> {
        co_return 1;
    };

    auto main = [&]() -> Task<> {
        int sum = 0;
        for (int i = 0; i < 1'000'000; ++i) {
            sum += co_await completes_synchronously();
        }
        assert(sum == 1'000'000);
    };

    auto run = [&](){
        asyncio::run(main());
    };
    TEST_CALL_COUNT(run, 20);
}
static void sched_simple_test() {
    auto main = [&]() -> Task<int> {
        co_return 1;
    };

    auto run = [&](){
        asyncio::run(main());
    };
    TEST_CALL_COUNT(run, 1000);
}

int main() {
    TEST_CALL(lots_of_synchronous_completions);
    TEST_CALL(sched_simple_test);
    return 0;
}