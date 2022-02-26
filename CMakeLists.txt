cmake_minimum_required(VERSION 3.0)

project(Wingine)


if (NOT WIN32)
  add_definitions(-Wall -g -Wno-class-memaccess -Wno-init-list-lifetime -mavx -fPIC -DDEBUG)
endif()

set(SPURV_ROOT "${CMAKE_HOME_DIRECTORY}/../spurv")
set(HCONLIB_ROOT "${CMAKE_HOME_DIRECTORY}/../HConLib")
set(WINGINE_ROOT "${CMAKE_HOME_DIRECTORY}")
set(WINGINE_SRC_DIR "${WINGINE_ROOT}/src")
set(WINGINE_EXAMPLES_DIR "${WINGINE_ROOT}/examples")
set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
set(GENERELLE_ROOT "${CMAKE_HOME_DIRECTORY}/../generelle")
set(GENERELLE_INCLUDE_DIR "${GENERELLE_ROOT}/include")
set(GENERELLE_LIB_DIR "${GENERELLE_ROOT}/lib")
set(FLAWED_ROOT "${CMAKE_HOME_DIRECTORY}/../flawed")
set(FLAWED_INCLUDE_DIR "${FLAWED_ROOT}/include")
set(FLAWED_LIB_DIR "${FLAWED_ROOT}/lib")

set(WGUTIL_SOURCES camera.cpp model.cpp)
list(TRANSFORM WGUTIL_SOURCES PREPEND "WgUtils/")
set(WINGINE_SOURCES
  buffer.cpp
  image.cpp
  renderfamily.cpp
  Wingine.cpp
  framebuffer.cpp
  pipeline.cpp
  resource.cpp
  util.cpp
  semaphore.cpp
  CompatibleRenderPassRegistry.cpp
  VulkanInstanceManager.cpp
  DeviceManager.cpp
  QueueManager.cpp
  CommandManager.cpp
  log.cpp
  queueUtils.cpp
  deviceEvaluation.cpp
  ${WGUTIL_SOURCES})
list(TRANSFORM WINGINE_SOURCES PREPEND "${WINGINE_SRC_DIR}/")

if(WIN32)
  add_definitions(/std:c++latest /arch:AVX2)
  set(lib_search_path "${HCONLIB_ROOT}/lib/Debug")
  set(winval_libs Winval)
  set(VULKAN_LIB_NAME vulkan-1)
else()
  add_definitions(-std=c++2a -mavx)
  set(lib_search_path "${HCONLIB_ROOT}/lib")
  set(x_libs X11)
  set(winval_libs Winval ${x_libs} dl)
  set(VULKAN_LIB_NAME vulkan)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-delete-non-virtual-dtor")
endif()

set(common_libs ${VULKAN_LIB_NAME} ${winval_libs} FlatAlg spurv flawed)
set(all_libs ${common_libs} Wingine generelle)

include_directories(${HCONLIB_ROOT}/include
	${SPURV_ROOT}/include ${VULKAN_INCLUDE_DIR} ${GENERELLE_INCLUDE_DIR} ${FLAWED_INCLUDE_DIR})


if(WIN32)
    link_directories($ENV{VULKAN_SDK}/lib)
endif()

link_directories(${HCONLIB_ROOT}/lib)
link_directories(${SPURV_ROOT}/lib)
link_directories(${GENERELLE_LIB_DIR})
link_directories(${FLAWED_LIB_DIR})

add_library(Wingine STATIC ${WINGINE_SOURCES})

add_executable(trivial ${WINGINE_EXAMPLES_DIR}/trivial.cpp)
add_executable(test ${WINGINE_EXAMPLES_DIR}/test.cpp)
add_executable(depth_test ${WINGINE_EXAMPLES_DIR}/depth_test.cpp)
add_executable(texture_test ${WINGINE_EXAMPLES_DIR}/texture_test.cpp)
add_executable(model_viewer ${WINGINE_EXAMPLES_DIR}/model_viewer.cpp)
add_executable(instance_test ${WINGINE_EXAMPLES_DIR}/instance_test.cpp)
add_executable(frag ${WINGINE_EXAMPLES_DIR}/frag.cpp)

target_link_libraries(Wingine ${common_libs})

target_link_libraries(trivial ${all_libs})
target_link_libraries(test ${all_libs})
target_link_libraries(depth_test ${all_libs})
target_link_libraries(texture_test ${all_libs})
target_link_libraries(model_viewer ${all_libs})
target_link_libraries(instance_test ${all_libs})
target_link_libraries(frag ${all_libs})
