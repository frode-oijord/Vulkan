cmake_minimum_required (VERSION 3.15)
project (server LANGUAGES C CXX)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../)

if (WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif (WIN32)
find_package(Boost REQUIRED program_options)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(server server.cpp)
target_link_libraries (server ${Boost_LIBRARIES})

set_target_properties(server PROPERTIES CXX_STANDARD 20)
set_property(TARGET server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")


add_executable(client client.cpp)
target_link_libraries (client ${Boost_LIBRARIES})

set_target_properties(client PROPERTIES CXX_STANDARD 20)
set_property(TARGET client PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
