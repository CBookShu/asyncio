//
// Created by netcan on 2021/09/06.
//
#include <iostream>
#include <string_view>
#include <string>
#include <asyncio/task.h>
#include <asyncio/runner.h>
#include <asyncio/sleep.h>
#include <asyncio/schedule_task.h>
#include <asyncio/callstack.h>
using namespace ASYNCIO_NS;

Task<std::string_view> hello() {
    co_return "hello";
}

Task<std::string_view> world() {
    co_return "world";
}

Task<std::string> hello_world() {
    co_return std::string(co_await hello()) + " " + std::string(co_await world());
}

int main() {
    printf("run result: %s\n", asyncio::run(hello_world()).c_str());
    return 0;
}
