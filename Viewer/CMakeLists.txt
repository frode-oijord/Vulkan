cmake_minimum_required (VERSION 3.15)
project (Viewer LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_FORCE_DEPTH_ZERO_TO_ONE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_FORCE_LEFT_HANDED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_ENABLE_BETA_EXTENSIONS ")

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
endif(WIN32)

include_directories(${PROJECT_SOURCE_DIR}/../../glm)
include_directories(${PROJECT_SOURCE_DIR}/../../gli)
include_directories(${PROJECT_SOURCE_DIR}/../)
include_directories(${PROJECT_SOURCE_DIR})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

if (WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif (WIN32)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(
	Viewer 
	main.cpp 
	Window.h
	${PROJECT_SOURCE_DIR}/../Innovator/Defines.h
	${PROJECT_SOURCE_DIR}/../Innovator/Factory.h
	${PROJECT_SOURCE_DIR}/../Innovator/ScmEnv.h
	${PROJECT_SOURCE_DIR}/../Innovator/Nodes.h
	${PROJECT_SOURCE_DIR}/../Innovator/State.h
	${PROJECT_SOURCE_DIR}/../Innovator/Timer.h
	${PROJECT_SOURCE_DIR}/../Innovator/Visitor.cpp
	${PROJECT_SOURCE_DIR}/../Innovator/Visitor.h
	${PROJECT_SOURCE_DIR}/../Innovator/VulkanAPI.h)

set_property(TARGET Viewer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)
set_target_properties(Viewer PROPERTIES CXX_STANDARD 20)

target_link_libraries(Viewer $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib)
target_link_libraries (Viewer ${Boost_LIBRARIES})