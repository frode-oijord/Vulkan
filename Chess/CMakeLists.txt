cmake_minimum_required (VERSION 3.15)
project (chess LANGUAGES CXX)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

add_executable(repl Repl.cpp)
set_property(TARGET repl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(repl PROPERTIES CXX_STANDARD 20)

add_executable(test Test.cpp Chess.h)
set_property(TARGET test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test PROPERTIES CXX_STANDARD 20)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})
