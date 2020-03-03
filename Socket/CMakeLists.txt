cmake_minimum_required (VERSION 3.15)
project (sockets LANGUAGES CXX)

add_executable(sockets sockets.cpp)
set_property(TARGET sockets PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

set_target_properties(sockets PROPERTIES CXX_STANDARD 17)
include_directories($ENV{BOOST_ROOT})
include_directories(${PROJECT_SOURCE_DIR})
target_link_libraries(sockets $ENV{BOOST_ROOT}/stage/lib/libboost_date_time-vc142-mt-gd-x64-1_71.lib)
target_link_libraries(sockets $ENV{BOOST_ROOT}/stage/lib/libboost_regex-vc142-mt-gd-x64-1_71.lib)