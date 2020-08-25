cmake_minimum_required (VERSION 3.15)
project (server LANGUAGES C CXX)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

include_directories(${PROJECT_SOURCE_DIR})

find_package(Boost COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Threads REQUIRED)

add_executable(server main.cpp)
target_link_libraries (server ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

set_target_properties(server PROPERTIES CXX_STANDARD 20)
set_property(TARGET server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
