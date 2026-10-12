cmake_minimum_required(VERSION 3.20)

project(Huynity LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
option(HUYNITY_BUILD_EXAMPLES "Build standalone examples" ON)

add_subdirectory(engine/math)
add_subdirectory(engine/objectlib)

if(HUYNITY_BUILD_EXAMPLES)
    add_subdirectory(examples/objectlib)
endif()
