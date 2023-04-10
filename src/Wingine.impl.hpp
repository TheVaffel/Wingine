#pragma once

#include "./Wingine.hpp"

#include "./resource/BasicUniform.hpp"
#include "./resource/BasicUniformChain.hpp"
#include "./buffer/InternallyStagedVertexBuffer.hpp"
#include "./buffer/InternallyStagedStorageBuffer.hpp"
#include "./resource/BasicResourceSetChain.hpp"
#include "./resource/descriptorUtil.hpp"

namespace wg {

    namespace {

        template<typename T>
        struct ResourceType { };

        template<typename T>
        struct ResourceType <std::shared_ptr<internal::IUniformChain<T>>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eUniformBuffer;
        };

        template<>
        struct ResourceType <std::shared_ptr<internal::ITextureChain>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eCombinedImageSampler;
        };

        template<>
        struct ResourceType <std::shared_ptr<internal::StorageTexture::TextureResourceComponent>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eCombinedImageSampler;
        };

        template<>
        struct ResourceType <std::shared_ptr<internal::StorageTexture::StorageImageResourceComponent>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eStorageImage;
        };


        template<typename T>
        struct ResourceType <std::shared_ptr<internal::IStorageBuffer<T>>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eStorageBuffer;
        };

        template<>
        struct ResourceType <std::shared_ptr<wg::internal::FramebufferTextureChain>> {
            constexpr static vk::DescriptorType value = vk::DescriptorType::eCombinedImageSampler;
        };

        template<typename T, typename... Ts>
        void getDescriptorSetBindings(std::vector<vk::DescriptorSetLayoutBinding>& bindings,
                                      T resource, Ts... rest) {

            vk::DescriptorSetLayoutBinding binding;
            binding.setBinding(bindings.size())
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eAll)
                .setDescriptorType(ResourceType<T>::value);

            bindings.push_back(binding);

            if constexpr (sizeof...(Ts) > 0) {
                getDescriptorSetBindings(bindings, rest...);
            }
        }

        template<typename... Ts>
        vk::DescriptorSetLayout createLayout(const vk::Device& device,
                                             Ts... resources) {
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            if constexpr (sizeof...(Ts) > 0) {
                getDescriptorSetBindings(bindings, resources...);
            }

            return
                internal::descriptorUtil::createDescriptorSetLayoutFromBindings(bindings,
                                                                                device);
        }
    }

    template<typename T>
    UniformPtr<T> Wingine::createUniform() {
        return std::make_shared<internal::BasicUniform<T>>(this->device_manager);
    }

    template<typename T>
    UniformChainPtr<T> Wingine::createUniformChain() {
        auto uniform_chain = std::make_shared<internal::BasicUniformChain<T>>(this->getNumFramebuffers(),
                                                                              this->device_manager);
        this->current_chain_reel->addChain(uniform_chain);
        return uniform_chain;
    }

    template<typename T>
    VertexBufferPtr<T> Wingine::createVertexBuffer(uint32_t element_count) {
        return std::make_shared<internal::InternallyStagedVertexBuffer<T>>(element_count,
                                                                           this->device_manager,
                                                                           this->queue_manager,
                                                                           this->command_manager);
    }

    template <typename T>
    StorageBufferPtr<T> Wingine::createStorageBuffer(uint32_t element_count) {
        return std::make_shared<internal::InternallyStagedStorageBuffer<T>>(element_count,
                                                                            this->device_manager,
                                                                            this->queue_manager,
                                                                            this->command_manager);
    }

    template<typename... Ts>
    ResourceSetChainPtr Wingine::createResourceSetChain(Ts... resources) {
        vk::DescriptorSetLayout layout = createLayout(this->device_manager->getDevice(), resources...);
        ResourceSetChainPtr res = std::make_shared<internal::BasicResourceSetChain>(
            this->getNumFramebuffers(),
            layout,
            this->descriptor_pool,
            this->device_manager,
            resources...);
        this->current_chain_reel->addChain(res);
        return res;
    }
};
