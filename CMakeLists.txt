cmake_minimum_required(VERSION 4.0)
project(Network_traffic_monitoring_system C)

set(CMAKE_C_STANDARD 11)

add_executable(Network_traffic_monitoring_system main.c
        include/graph.h
        include/traffic.h
        include/utils.h
        core/main.cpp
        core/graph.cpp
        core/traffic.cpp
        core/path.cpp
        core/utils.cpp
        include/path.h)
