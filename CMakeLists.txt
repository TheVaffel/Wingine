cmake_minimum_required(VERSION 3.0)

project(Wingine)


if (NOT WIN32)
  add_definitions(-Wall -g -Wno-class-memaccess -mavx)
endif()

set(SPURV_ROOT "${CMAKE_HOME_DIRECTORY}/../spurv")
set(HCONLIB_ROOT "${CMAKE_HOME_DIRECTORY}/../HConLib")
set(WINGINE_ROOT "${CMAKE_HOME_DIRECTORY}")
set(WINGINE_SRC_DIR "${WINGINE_ROOT}/src")
set(WINGINE_EXAMPLES_DIR "${WINGINE_ROOT}/examples")
set(WINGINE_INCLUDE_DIR "${WINGINE_ROOT}/include")

set(WINGINE_SOURCES buffer.cpp image.cpp renderfamily.cpp Wingine.cpp framebuffer.cpp pipeline.cpp resource.cpp util.cpp WgUtils.cpp)
list(TRANSFORM WINGINE_SOURCES PREPEND "${WINGINE_SRC_DIR}/")

if(WIN32)
  add_definitions(/std:c++latest)
  set(lib_search_path "${HCONLIB_ROOT}/lib/Debug")
  set(winval_libs Winval)
  set(VULKAN_LIB_NAME vulkan-1)
else()
  add_definitions(-std=c++1z -fconcepts -mavx)
  set(lib_search_path "${HCONLIB_ROOT}/lib")
  set(x_libs X11)
  set(winval_libs Winval ${x_libs} dl)
  set(VULKAN_LIB_NAME vulkan)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-delete-non-virtual-dtor")
endif()

set(common_libs ${VULKAN_LIB_NAME} ${winval_libs} FlatAlg spurv)
set(all_libs ${common_libs} Wingine)

include_directories(${WINGINE_ROOT} ${HCONLIB_ROOT}/include ${SPURV_ROOT}/include $ENV{VULKAN_SDK}/include)


if(WIN32)
    link_directories($ENV{VULKAN_SDK}/lib)
else()
    link_directories($ENV{VULKAN_SDK}/lib)
endif()

link_directories(${HCONLIB_ROOT}/lib)
link_directories(${SPURV_ROOT}/lib)
include_directories(${WINGINE_INCLUDE_DIR})

add_library(Wingine STATIC ${WINGINE_SOURCES})

add_executable(test ${WINGINE_EXAMPLES_DIR}/test.cpp)
add_executable(depth_test ${WINGINE_EXAMPLES_DIR}/depth_test.cpp)
add_executable(texture_test ${WINGINE_EXAMPLES_DIR}/texture_test.cpp)

target_link_libraries(Wingine ${common_libs})

target_link_libraries(test ${all_libs})
target_link_libraries(depth_test ${all_libs})
target_link_libraries(texture_test ${all_libs})
