cmake_minimum_required(VERSION 3.14)
project(tentris_bench CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

add_executable(tentris-bench
    src/main.cpp
)

target_include_directories(tentris-bench PRIVATE include)
target_link_libraries(tentris-bench CURL::libcurl)