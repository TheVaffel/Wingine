
#ifndef WG_DECLARATIONS_HPP
#define WG_DECLARATIONS_HPP

#include <Winval.hpp>

#include <vulkan/vulkan.h>
#ifdef WIN32

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan_win32.h"

#include <algorithm>

#undef min
#undef max

#else // WIN32
#include <unistd.h>

#define VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan_xlib.h"

#endif // WIN32

#include <vulkan/vulkan.hpp>

#define DEBUG

namespace wg {
  
  // For vertex buffers
  enum ComponentType
    {
     tFloat32,
     tFloat64,
     tInt32,
     tInt64
    };
  
  struct VertexAttribDesc {
    ComponentType component_type;
    uint32_t binding_num;
    uint32_t num_elements;
    uint32_t stride_in_bytes;
    uint32_t offset_in_bytes;
    bool per_instance = false;
  };

  // Uniform template
  template<typename Type>
  class Uniform;
  
  enum ResourceType {
    resUniform = (uint64_t)VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    resStorageBuffer = (uint64_t)VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    resTexture = (uint64_t)VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    resImage = (uint64_t)VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
  };

  // These are shifted, because we want to combine them
  // with resource types as a bitmask
  /* enum ShaderStage {
      // shaVertex = ((long long unsigned int)VK_SHADER_STAGE_VERTEX_BIT) << 32LL, 
      // shaFragment = ((long long unsigned int)VK_SHADER_STAGE_FRAGMENT_BIT) << 32LL
  }; */
  // Also, turns out we can't really assume compilers support 64-bit
  // enum values (looking at you, Windows), so we'll go the cheap way
  // and declare them as integers instead
  typedef int64_t ShaderType;
  const ShaderType shaVertex = (int64_t)VK_SHADER_STAGE_VERTEX_BIT << 32;
  const ShaderType shaFragment = (int64_t)VK_SHADER_STAGE_FRAGMENT_BIT << 32;
  const ShaderType shaCompute = (int64_t)VK_SHADER_STAGE_COMPUTE_BIT << 32;
  
  enum ImageViewType {
    wImageViewColor,
    wImageViewDepth
  };

  enum RenderPassType {
    renColorDepth,
    renDepth
  };

  class Wingine;
  class Resource;
  class RenderFamily;
  class Framebuffer;
  class Texture;
  class Image;
  class Buffer;
  class SemaphoreChain;
  class ResourceImage;
  
  template<typename tt>
  class VertexBuffer;
  
  class IndexBuffer;
  class StorageBuffer;
};

#endif // WG_DECLARATIONS_HPP
