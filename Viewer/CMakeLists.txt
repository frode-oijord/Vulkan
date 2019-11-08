cmake_minimum_required (VERSION 3.15)
project (Viewer LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_executable(Viewer main.cpp 
	${PROJECT_SOURCE_DIR}/../Innovator/Nodes.cpp
	${PROJECT_SOURCE_DIR}/../Innovator/Visitor.cpp
	${PROJECT_SOURCE_DIR}/../Innovator/EventVisitor.cpp)	

set_property(TARGET Viewer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

set_target_properties(Viewer PROPERTIES CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_FORCE_DEPTH_ZERO_TO_ONE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_FORCE_LEFT_HANDED")

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
endif(WIN32)

include_directories($ENV{BOOST_ROOT})
include_directories(${PROJECT_SOURCE_DIR}/../../glm)
include_directories(${PROJECT_SOURCE_DIR}/../../gli)
include_directories(${PROJECT_SOURCE_DIR}/../)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Vulkan_INCLUDE_DIRS})

target_link_libraries(Viewer ${Vulkan_LIBRARIES})
target_link_libraries(Viewer $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib)
