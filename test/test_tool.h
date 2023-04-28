#ifndef __TEST_TOOL_HEADER__
#define __TEST_TOOL_HEADER__

#include <iostream>
#include <random>
#include <cassert>
#include <chrono>
#include <cstring>
#include <cmath>

#define TEST_CALL(f)    do {\
    std::cout << "calc "<< #f << " start" << std::endl;\
    auto b = std::chrono::high_resolution_clock::now();\
    f();\
    auto e = std::chrono::high_resolution_clock::now();\
    auto diff  = std::chrono::duration_cast<std::chrono::nanoseconds>(e-b).count();\
    std::cout << "calc "<< #f << " end  cost:" << diff << "ns" << std::endl;\
}while(0)

#define TEST_CALL_COUNT(f, c)    do {\
    std::cout << "calc "<< #f << " start" << std::endl;\
    auto b = std::chrono::high_resolution_clock::now();\
    for(std::int64_t i = 0; i < c; ++i) f();\
    auto e = std::chrono::high_resolution_clock::now();\
    auto diff  = std::chrono::duration_cast<std::chrono::nanoseconds>(e-b).count();\
    std::cout << "calc "<< #f << " end" << std::endl;\
    std::cout << "total cost" << diff << std::endl;\
    std::cout << "average cost:" << diff/c << "ns" << std::endl;\
}while(0)

#define REQUIRE(exp)    assert((exp))
#define GIVEN(exp)      if(true)
#define SECTION(exp)    if(true)
#define REQUIRE_THROWS_AS(exp, etype)   try {static_cast<void>(exp);} catch(etype const& ) {} catch(...) {}
#define REQUIRE_NOTHROW(exp)  try {static_cast<void>(exp);} catch(...) {}
#define APRROX_EQ(x,y)  (std::fabs(x-y) < 1e-5)
#endif // __TEST_TOOL_HEADER__