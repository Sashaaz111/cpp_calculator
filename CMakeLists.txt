cmake_minimum_required(VERSION 3.15)
project(PluginCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(calc
    src/main.cpp
    src/PluginManager.cpp
    src/ExpressionParser.cpp
)

if(UNIX AND NOT APPLE)
    target_link_libraries(calc PRIVATE dl)
endif()

add_subdirectory(plugins)
