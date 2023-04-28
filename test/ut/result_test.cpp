//
// Created by netcan on 2021/11/24.
//
#include <asyncio/result.h>
#include <asyncio/runner.h>
#include <asyncio/task.h>
#include "counted.h"
#include "../test_tool.h"

using namespace ASYNCIO_NS;

static void test_Counted() {
    using TestCounted = Counted<default_counted_policy>;
    TestCounted::reset_count();
    auto f_move_counted = [&](){
        {
            TestCounted c1;
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    };
    TEST_CALL(f_move_counted);

    TestCounted::reset_count();
    auto f_copy_counted1 = [&]() {
        {
            TestCounted c1;
            TestCounted c2(c1);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    };
    TEST_CALL(f_copy_counted1);

    TestCounted::reset_count();
    auto f_copy_counted2 = [&]() {
        TestCounted c1;
        {
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(c1.id_ == -1);
    };
    TEST_CALL(f_copy_counted2);
}

static void test_result_T() {
    using TestCounted = Counted<CountedPolicy{
        .move_assignable=false,
        .copy_assignable=false
    }>;
    TestCounted::reset_count();
    auto f_result_set_lvalue = [&]() {
        Result<TestCounted> res;
        REQUIRE(! res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::copy_construct_counts == 0);
            res.set_value(c);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    };
    TEST_CALL(f_result_set_lvalue);

    TestCounted::reset_count();
    auto f_result_set_rvalue =[&]() {
        Result<TestCounted> res;
        REQUIRE(! res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::move_construct_counts == 0);
            res.set_value(std::move(c));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    };
    TEST_CALL(f_result_set_rvalue);

    TestCounted::reset_count();
    auto f_lvalue_result = [&]() {
        Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        {
            {
                auto&& r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::move_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }
            {
                auto r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 2);
            }
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    };
    TEST_CALL(f_lvalue_result);

    TestCounted::reset_count();
    auto f_rvalue_result =[&]() {
        Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        {
            auto r = std::move(res).result();
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    };
    TEST_CALL(f_rvalue_result);

}

static void test_Counted_for_Task() {
    using TestCounted = Counted<default_counted_policy>;
    TestCounted::reset_count();

    auto build_count = []() -> Task<TestCounted> {
        co_return TestCounted{};
    };
    bool called{false};

    auto f_return_a_counted= [&]() {
        asyncio::run([&]() -> Task<> {
            auto c = co_await build_count();
            REQUIRE(TestCounted::alive_counts() == 1);
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(TestCounted::default_construct_counts == 1);
            REQUIRE(TestCounted::copy_construct_counts == 0);
            called = true;
        }());
        REQUIRE(called);
    };
    TEST_CALL(f_return_a_counted);

    TestCounted::reset_count();
    auto f_return_a_lvalue_counted =[&]() {
        asyncio::run([&]() -> Task<> {
            auto t = build_count();
            {
                auto c = co_await t;
                REQUIRE(TestCounted::alive_counts() == 2);
                REQUIRE(TestCounted::move_construct_counts == 1);
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }

            {
                auto c = co_await std::move(t);
                REQUIRE(TestCounted::alive_counts() == 2);
                REQUIRE(TestCounted::move_construct_counts == 2);
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }

            called = true;
        }());
        REQUIRE(called);
    };
    TEST_CALL(f_return_a_lvalue_counted);

    TestCounted::reset_count();
    auto f_rvalue_task_get_result= [&]() {
        auto c = asyncio::run(build_count());
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(TestCounted::move_construct_counts == 2);
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
    };
    TEST_CALL(f_rvalue_task_get_result);

    TestCounted::reset_count();
    auto f_lvalue_task_get_result =[&]() {
        auto t = build_count();
        auto c = asyncio::run(t);
        REQUIRE(TestCounted::alive_counts() == 2);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 1);
    };
    TEST_CALL(f_lvalue_task_get_result);
}


static void test_pass_parameters_to_the_coroutine_frame() {
    using TestCounted = Counted<CountedPolicy{
        .move_assignable=false,
        .copy_assignable=false
    }>;
    TestCounted::reset_count();

    auto f_pass_by_rvalue = [&]() {
#if defined(_WIN32)
        // win32 这里临时的TestCounted会进行一次move；跟gcc表现不一致无法跑通该测试用例
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(count.alive_counts() == 1);
            co_return;
        };
        asyncio::run(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
#else
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(count.alive_counts() == 2);
            co_return;
        };
        asyncio::run(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
#endif
    };
    TEST_CALL(f_pass_by_rvalue);

    TestCounted::reset_count();
    auto f_pass_by_lvalue = [&]() {
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::move_construct_counts == 1);
#if defined(_WIN32)
            // win32 会在调用coro(count) 的时候，把中间无法获取到的将亡值自动释放
            REQUIRE(count.alive_counts() == 2);
#else
            REQUIRE(count.alive_counts() == 3);
#endif
            co_return;
        };
        TestCounted count;
        asyncio::run(coro(count));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id_ != -1);
    };
    TEST_CALL(f_pass_by_lvalue);

    TestCounted::reset_count();
    auto f_pass_by_xvalue =[&]() {
        auto coro = [](TestCounted count) -> Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 0);
            REQUIRE(TestCounted::move_construct_counts == 2);
#if defined(_WIN32)
            // win32 会在调用coro(count) 的时候，把中间无法获取到的将亡值自动释放
            REQUIRE(count.alive_counts() == 2);
#else
            REQUIRE(count.alive_counts() == 3);
#endif
            REQUIRE(count.id_ != -1);
            co_return;
        };
        TestCounted count;
        asyncio::run(coro(std::move(count)));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        REQUIRE(TestCounted::move_construct_counts == 2);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id_ == -1);
    };
    TEST_CALL(f_pass_by_xvalue);

    TestCounted::reset_count();
    auto f_pass_by_lvalue_ref = [&] {
        TestCounted count;
        auto coro = [&](TestCounted& cnt) -> Task<> {
            REQUIRE(cnt.alive_counts() == 1);
            REQUIRE(&cnt == &count);
            co_return;
        };
        asyncio::run(coro(count));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
    };
    TEST_CALL(f_pass_by_lvalue_ref);

    TestCounted::reset_count();
    auto f_pass_by_rvalue_ref =[&]() {
        auto coro = [](TestCounted&& count) -> Task<> {
            REQUIRE(count.alive_counts() == 1);
            co_return;
        };
        asyncio::run(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
    };
    TEST_CALL(f_pass_by_rvalue_ref);
}

int main() {
    TEST_CALL(test_Counted);
    TEST_CALL(test_result_T);
    TEST_CALL(test_Counted_for_Task);
    TEST_CALL(test_pass_parameters_to_the_coroutine_frame);
    return 0;
}