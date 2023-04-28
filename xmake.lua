set_languages("c++20")

target("asyncio")
    set_kind("static")
    add_includedirs("include/", {public = true})
    add_files("src/*.cpp")

target("sched_test")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/pt/sched_test.cpp")

target("echo_client")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/st/echo_client.cpp")

target("echo_server")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/st/echo_server.cpp")

target("hello_world")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/st/hello_world.cpp")

target("result_test")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/ut/result_test.cpp")

target("selector_test")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/ut/selector_test.cpp")

target("task_test")
    set_kind("binary")
    add_deps("asyncio")
    add_files("test/ut/task_test.cpp")

-- ut {result_test.cpp,selector_test.cpp,task_test.cpp}