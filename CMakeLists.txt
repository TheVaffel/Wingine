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

set(BUFFER_SOURCES
  bufferUtil.cpp
  IBuffer.cpp
  BasicBuffer.cpp
  InternallyStagedIndexBuffer.cpp
  IIndexBuffer.cpp
  StagingBuffer.cpp
  # StagingBufferManager.cpp
  copyBuffer.cpp)
set(RESOURCE_SOURCES
  descriptorUtil.cpp
  BasicResourceSet.cpp
  BasicResourceSetChain.cpp
  ResourceSetLayoutRegistry.cpp
  StaticResourceChain.cpp)
set(FRAMEBUFFER_SOURCES
  framebufferUtil.cpp
  IFramebuffer.cpp
  BasicFramebuffer.cpp
  BasicFramebufferSetup.cpp
  SwapchainFramebuffer.cpp
  IFramebufferChain.cpp
  HostCopyingFramebufferChain.cpp
  SwapchainFramebufferChain.cpp
  FramebufferChainBase.cpp
  FramebufferTexture.cpp
  FramebufferTextureChain.cpp)
set(IMAGE_SOURCES
  imageUtil.cpp
  IImage.cpp
  BasicImage.cpp
  SwapchainColorImage.cpp
  HostVisibleImageView.cpp
  ImageCopier.cpp
  ImageChainCopier.cpp
  ImageToBufferCopier.cpp
  copyImage.cpp
  BasicTexture.cpp
  BasicTextureChain.cpp
  samplerUtil.cpp
  StagingImage.cpp
  InternallyStagedTexture.cpp
  BasicImageSetup.cpp
  StorageTexture.cpp)
set(RENDER_PASS_SOURCES renderPassUtil.cpp CompatibleRenderPassRegistry.cpp)
set(SYNC_SOURCES
  semaphoreUtil.cpp
  ManagedSemaphoreChain.cpp
  SemaphoreSetBase.cpp
  SignalAndWaitSemaphores.cpp
  SynchronizedQueueOperationBase.cpp
  WaitSemaphoreSet.cpp
  SignalSemaphoreSet.cpp
  fenceUtil.cpp
  EventChain.cpp)
set(COMMAND_SOURCES
  CommandChainController.cpp
  recordUtil.cpp
  CommandControllerSettings.cpp)
set(PIPELINE_SOURCES
  BasicPipeline.cpp
  BasicPipelineSetup.cpp
  BasicShader.cpp
  pipelineUtil.cpp
  shaderUtil.cpp
  VertexAttribDesc.cpp
  BasicComputePipeline.cpp)
set(DRAW_PASS_SOURCES BasicDrawPass.cpp DrawPassBase.cpp)
set(MEMORY_SOURCES memoryUtil.cpp)
set(UTIL_SOURCES IndexCounter.cpp SettableIndexCounter.cpp log.cpp math.cpp)
set(WGUTIL_SOURCES camera.cpp model.cpp)

list(TRANSFORM BUFFER_SOURCES PREPEND "buffer/")
list(TRANSFORM FRAMEBUFFER_SOURCES PREPEND "framebuffer/")
list(TRANSFORM IMAGE_SOURCES PREPEND "image/")
list(TRANSFORM RENDER_PASS_SOURCES PREPEND "render_pass/")
list(TRANSFORM MEMORY_SOURCES PREPEND "memory/")
list(TRANSFORM SYNC_SOURCES PREPEND "sync/")
list(TRANSFORM DRAW_PASS_SOURCES PREPEND "draw_pass/")
list(TRANSFORM RESOURCE_SOURCES PREPEND "resource/")
list(TRANSFORM COMMAND_SOURCES PREPEND "command/")
list(TRANSFORM PIPELINE_SOURCES PREPEND "pipeline/")
list(TRANSFORM UTIL_SOURCES PREPEND "util/")
list(TRANSFORM WGUTIL_SOURCES PREPEND "WgUtils/")

set(WINGINE_SOURCES
  buffer.cpp
  image.cpp
  # renderfamily.cpp
  compute_pipeline.cpp
  Wingine.cpp
  resource.cpp
  util.cpp
  semaphore.cpp
  VulkanInstanceManager.cpp
  DeviceManager.cpp
  QueueManager.cpp
  CommandManager.cpp
  SwapchainManager.cpp
  queueUtils.cpp
  deviceEvaluation.cpp
  ${BUFFER_SOURCES}
  ${FRAMEBUFFER_SOURCES}
  ${IMAGE_SOURCES}
  ${RENDER_PASS_SOURCES}
  ${MEMORY_SOURCES}
  ${SYNC_SOURCES}
  ${DRAW_PASS_SOURCES}
  ${RESOURCE_SOURCES}
  ${COMMAND_SOURCES}
  ${PIPELINE_SOURCES}
  ${UTIL_SOURCES}
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

set(common_libs ${VULKAN_LIB_NAME} ${winval_libs} FlatAlg HGraf spurv flawed OpenImageIO OpenImageIO_Util)
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
add_executable(offscreen ${WINGINE_EXAMPLES_DIR}/offscreen.cpp ${WINGINE_EXAMPLES_DIR}/util/renderdoc_util.cpp)

target_compile_definitions(offscreen PUBLIC HEADLESS)

target_link_libraries(Wingine ${common_libs})

target_link_libraries(trivial ${all_libs})
target_link_libraries(test ${all_libs})
target_link_libraries(depth_test ${all_libs})
target_link_libraries(texture_test ${all_libs})
target_link_libraries(model_viewer ${all_libs})
target_link_libraries(instance_test ${all_libs})
target_link_libraries(frag ${all_libs})
target_link_libraries(offscreen ${all_libs})
