cmake_minimum_required (VERSION 3.15)
project (generator LANGUAGES CXX)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

include_directories(${PROJECT_SOURCE_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/../../glm)
include_directories(${PROJECT_SOURCE_DIR}/../../gli)

link_directories(${Boost_LIBRARY_DIRS})

add_executable(generator main.cpp)
set_target_properties(generator PROPERTIES CXX_STANDARD 17)
set_property(TARGET generator PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")