cmake_minimum_required(VERSION 3.16)
project(Network_traffic_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Network_traffic_monitoring_system
    main.c
    core/graph.cpp
    core/traffic.cpp
    core/path.cpp
    core/utils.cpp
)

set_source_files_properties(main.c PROPERTIES LANGUAGE CXX)

target_include_directories(Network_traffic_monitoring_system PRIVATE include)
