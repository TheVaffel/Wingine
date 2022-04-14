#include "./DeviceManager.hpp"

#include "./deviceEvaluation.hpp"
#include "./queueUtils.hpp"
#include "./log.hpp"

#include <iostream>

namespace wg::internal {
    namespace {
        void printDeviceName(const vk::PhysicalDevice& physical_device) {

            vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceVulkan12Properties> props =
                physical_device.getProperties2<vk::PhysicalDeviceProperties2,
                                               vk::PhysicalDeviceVulkan12Properties>();

            vk::PhysicalDeviceProperties2 props2 = props.get<vk::PhysicalDeviceProperties2>();

            std::cout << "Device name: " << props2.properties.deviceName << std::endl;
        }
    };

    DeviceManager::DeviceManager(std::shared_ptr<VulkanInstanceManager> vulkan_instance_manager)
        : vulkan_instance_manager(vulkan_instance_manager) {

        vk::Instance instance = vulkan_instance_manager->getInstance();

        std::vector<vk::PhysicalDevice> found_devices = instance.enumeratePhysicalDevices();

        std::cout << "Number of devices: " << found_devices.size() << std::endl;

        uint32_t max_score = 0;
        vk::PhysicalDevice best_device;

        for(vk::PhysicalDevice dev : found_devices) {
            printDeviceName(dev);
            uint32_t new_score;
            if (vulkan_instance_manager->hasSurface()) {
                new_score = evaluatePhysicalDevice(dev, vulkan_instance_manager->getSurface());
            } else {
                new_score = evaluatePhysicalDeviceWithoutSurface(dev);
            }

            if (new_score > max_score) {
                max_score = new_score;
                best_device = dev;
            }
        }


        if (max_score == 0) {
            _wlog_error("Could not find device supporting both presenting and graphics");
        }

        this->physical_device = best_device;
        this->device_memory_props = best_device.getMemoryProperties();

        QueueIndices queue_indices;
        if (vulkan_instance_manager->hasSurface()) {
            queue_indices = getQueueIndicesForDevice(best_device,
                                                     vulkan_instance_manager->getSurface());
        } else {
            queue_indices = getQueueIndicesForDeviceWithoutSurface(best_device);
        }

        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos =
            getDeviceQueueCreateInfos(queue_indices);

        std::vector<const char*> device_extension_names;
        device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vk::PhysicalDeviceFeatures feats = {};
        feats.setShaderClipDistance(VK_TRUE)
            .setFillModeNonSolid(VK_TRUE);

        vk::PhysicalDeviceVulkan12Features feats12;
        feats12.setTimelineSemaphore(VK_TRUE);

        vk::DeviceCreateInfo device_info;
        device_info.setQueueCreateInfoCount(queue_create_infos.size())
            .setPQueueCreateInfos(queue_create_infos.data())
            .setEnabledExtensionCount(device_extension_names.size())
            .setPpEnabledExtensionNames(device_extension_names.data())
            .setEnabledLayerCount(0)
            .setPpEnabledLayerNames(nullptr)
            .setPEnabledFeatures(&feats)
            .setPNext(&feats12);

        this->device = this->physical_device.createDevice(device_info);
    }

    DeviceManager::~DeviceManager() {



        this->device.destroy();
    }

    const vk::Device DeviceManager::getDevice() const {
        return this->device;
    }

    const vk::PhysicalDevice DeviceManager::getPhysicalDevice() const {
        return this->physical_device;
    }

    const vk::PhysicalDeviceMemoryProperties DeviceManager::getDeviceMemoryProperties() const {
        return this->device_memory_props;
    }
};
