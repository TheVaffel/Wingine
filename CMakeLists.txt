cmake_minimum_required(VERSION 3.0)

project(Wingine)


if (NOT WIN32)
  add_definitions(-Wall -g -Wno-class-memaccess)
endif()

set(SPURV_ROOT "${CMAKE_HOME_DIRECTORY}/../spurv")
set(HCONLIB_ROOT "${CMAKE_HOME_DIRECTORY}/../HConLib")
set(WINGINE_ROOT "${CMAKE_HOME_DIRECTORY}")


if(WIN32)
  add_definitions(/std:c++latest)
  set(lib_search_path "${HCONLIB_ROOT}/lib/Debug")
  set(winval_libs Winval)
  set(VULKAN_LIB_NAME vulkan-1)
else()
  add_definitions(-std=c++1z -fconcepts)
  set(lib_search_path "${HCONLIB_ROOT}/lib")
  set(x_libs X11)
  set(winval_libs Winval ${x_libs})
  set(VULKAN_LIB_NAME vulkan)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-delete-non-virtual-dtor")
endif()

set(common_libs ${VULKAN_LIB_NAME} ${winval_libs} FlatAlg spurv)

include_directories(${WINGINE_ROOT} ${HCONLIB_ROOT}/include ${SPURV_ROOT}/include $ENV{VULKAN_SDK}/include)


if(WIN32)
    link_directories($ENV{VULKAN_SDK}/lib)
else()
    link_directories($ENV{VULKAN_SDK}/lib)
endif()

link_directories(${HCONLIB_ROOT}/lib)
link_directories(${SPURV_ROOT}/lib)

add_executable(test test.cpp Wingine.cpp WgUtils.cpp)
add_executable(depth_test depth_test.cpp Wingine.cpp WgUtils.cpp)
add_executable(texture_test texture_test.cpp Wingine.cpp WgUtils.cpp)

target_link_libraries(test ${common_libs})
target_link_libraries(depth_test ${common_libs})
target_link_libraries(texture_test ${common_libs})
