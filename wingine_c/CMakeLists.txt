project(wingine_c)

cmake_minimum_required(VERSION 3.0)

if(WIN32)
else()
  add_definitions(-std=c++2a -mavx)
endif()

set(HCONLIB_ROOT
  ${CMAKE_HOME_DIRECTORY}/../../HConLib)

set(HCONLIB_INCLUDE ${HCONLIB_ROOT}/include)
set(HCONLIB_LIB ${HCONLIB_ROOT}/lib)

set(FLAWED_ROOT
  ${CMAKE_HOME_DIRECTORY}/../../flawed)
set(FLAWED_INCLUDE ${FLAWED_ROOT}/include)
set(FLAWED_LIB ${FLAWED_ROOT}/lib)

set(LIBS Wingine FlatAlg Winval X11 flawed vulkan)

set(WINGINE_SRC
  src/wg_wingine.cpp
  src/wg_buffer.cpp)

include_directories(../include ${HCONLIB_INCLUDE} ${FLAWED_INCLUDE})
link_directories(../build ${HCONLIB_LIB} ${FLAWED_LIB})

add_library(wingine_c STATIC ${WINGINE_SRC})

add_subdirectory(examples)

target_link_libraries(wingine_c ${LIBS})
