cmake_minimum_required(VERSION 4.1)
project(Multi_Asset_Monte_Carlo_Engine)

set(CMAKE_CXX_STANDARD 23)

add_executable(Multi_Asset_Monte_Carlo_Engine
        src/main.cpp
        src/MultiAssetSimulator.cpp)

find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
    target_link_libraries(Multi_Asset_Monte_Carlo_Engine PUBLIC OpenMP::OpenMP_CXX)
endif()
