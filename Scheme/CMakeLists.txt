cmake_minimum_required (VERSION 3.15)
project (scheme LANGUAGES CXX)

add_executable(scheme Repl.cpp)
set_property(TARGET scheme PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)
set_target_properties(scheme PROPERTIES CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR})
