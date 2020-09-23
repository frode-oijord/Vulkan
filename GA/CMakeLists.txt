cmake_minimum_required (VERSION 3.15)
project (ga LANGUAGES CXX)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

add_executable(test_r2 test_r2.cpp r2.h)
set_property(TARGET test_r2 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_r2 PROPERTIES CXX_STANDARD 20)

add_executable(test_pga3d test_pga3d.cpp pga3d.h)
set_property(TARGET test_pga3d PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_pga3d PROPERTIES CXX_STANDARD 20)

add_executable(pga3d pga3d.cpp)
set_property(TARGET pga3d PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(pga3d PROPERTIES CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR})
