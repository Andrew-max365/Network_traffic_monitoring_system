cmake_minimum_required(VERSION 3.16)
project(Network_traffic_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

add_executable(Network_traffic_monitoring_system
    core/main.cpp
    core/graph.cpp
    core/traffic.cpp
    core/path.cpp
    core/utils.cpp
    include/dsu.h
    core/dsu.cpp
)

target_include_directories(Network_traffic_monitoring_system PRIVATE include)
