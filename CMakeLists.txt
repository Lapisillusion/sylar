cmake_minimum_required(VERSION 3.27)
project(sylar)

set(CMAKE_CXX_FLAGS "-rdynamic -O0 -ggdb -Wall -Werror")

set(CMAKE_CXX_STANDARD 17)
find_package(yaml-cpp REQUIRED)
add_executable(sylar main.cpp
        include/util.h
        include/singleton.h
        include/noncopyable.h
        include/mutex.h
        include/macro.h
        include/log.h
        src/log/log.cpp
        src/util/util.cpp
        tests/test_log.cpp
        include/sylar.h
        include/test.h
        tests/test_util.cpp
        include/env.h
        src/env/env.cpp
        tests/test_env.cpp
        include/config.h
        src/config/config.cpp
        tests/test_config.cpp)

target_link_libraries(sylar yaml-cpp)