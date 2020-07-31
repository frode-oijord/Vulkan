cmake_minimum_required(VERSION 3.4.1)

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

add_library (boost_system SHARED IMPORTED)

set(MY_BOOST_LIBS_DIR ndk_21_boost_1.72.0/libs)
set(MY_BOOST_INC_DIR ndk_21_boost_1.72.0/include)

set_target_properties(boost_system PROPERTIES IMPORTED_LOCATION
        ${MY_BOOST_LIBS_DIR}/${ANDROID_ABI}/libboost_system.so)

add_library(boost_chrono SHARED IMPORTED)

set_target_properties(boost_chrono PROPERTIES IMPORTED_LOCATION
        ${MY_BOOST_LIBS_DIR}/${ANDROID_ABI}/libboost_chrono.so)

# build vulkan app
set(SRC_DIR src/main/jni)
set(INNOVATOR_DIR ../../)
set(GLM_DIR ../../../glm)
set(GLI_DIR ../../../gli)

add_library(vktuts SHARED
            ${SRC_DIR}/AndroidMain.cpp
            ${INNOVATOR_DIR}/Innovator/Visitor.cpp)

include_directories(${INNOVATOR_DIR} ${GLM_DIR} ${GLI_DIR} ${MY_BOOST_INC_DIR})

set_target_properties(vktuts PROPERTIES CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -DVK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
target_link_libraries(vktuts app-glue log android)