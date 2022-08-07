#include "./queueUtils.hpp"

#include <iostream>

namespace wg::internal {
    namespace {
        bool queueFamilySupportsGraphics(vk::QueueFamilyProperties props) {
            return (props.queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlagBits {};
        }

        bool queueFamilySupportsCompute(vk::QueueFamilyProperties props) {
            return (props.queueFlags & vk::QueueFlagBits::eCompute) != vk::QueueFlagBits {};
        }

        void setSupportedIndices(vk::PhysicalDevice physical_device,
                                 vk::SurfaceKHR surface,
                                 vk::QueueFamilyProperties qprops,
                                 int index,
                                 QueueIndices* indices) {

            if (queueFamilySupportsGraphics(qprops)) {
                indices->graphics = index;
            }

            vk::Bool32 supportsPresent = physical_device.getSurfaceSupportKHR(index, surface);
            if (supportsPresent) {
                indices->present = index;
            }

            if (queueFamilySupportsCompute(qprops)) {
                indices->compute = index;
            }
        }

        void setSupportedIndicesWithoutSurface(vk::PhysicalDevice physical_device,
                                               vk::QueueFamilyProperties qprops,
                                               int index,
                                               QueueIndices* indices) {
            if (queueFamilySupportsGraphics(qprops)) {
                indices->graphics = index;
            }

            if (queueFamilySupportsCompute(qprops)) {
                indices->compute = index;
            }
        }
    };

    QueueIndices getQueueIndicesForDeviceWithoutSurface(vk::PhysicalDevice physical_device) {
        QueueIndices indices;

        std::vector<vk::QueueFamilyProperties> qprops = physical_device.getQueueFamilyProperties();
        for (unsigned int i = 0; i < qprops.size(); i++) {
            setSupportedIndicesWithoutSurface(physical_device,
                                              qprops[i],
                                              i,
                                              &indices);
        }

        return indices;
    }

    QueueIndices getQueueIndicesForDevice(vk::PhysicalDevice physical_device,
                                          vk::SurfaceKHR surface) {
        QueueIndices indices;

        std::vector<vk::QueueFamilyProperties> qprops = physical_device.getQueueFamilyProperties();
        for (unsigned int i = 0; i < qprops.size(); i++) {
            setSupportedIndices(physical_device,
                                surface,
                                qprops[i],
                                i,
                                &indices);
        }

        return indices;
    }

    std::vector<vk::DeviceQueueCreateInfo> getDeviceQueueCreateInfos(const QueueIndices& indices) {

        std::vector<vk::DeviceQueueCreateInfo> c_infos;
        c_infos.reserve(3);

        vk::DeviceQueueCreateInfo c_info;
        float queue_priorities[1] = { 1.0f };
        c_info.setQueueCount(1).setPQueuePriorities(queue_priorities)
            .setQueueFamilyIndex(indices.graphics);
        c_infos.push_back(c_info);

        if (indices.present >= 0 && indices.present != indices.graphics) {
            c_info.setQueueFamilyIndex(indices.present);
            c_infos.push_back(c_info);
        }

        if (indices.compute >= 0) {
            if (indices.compute != indices.present &&
                indices.compute != indices.graphics) {

                c_info.setQueueFamilyIndex(indices.compute);
                c_infos.push_back(c_info);
            }
        }

        return c_infos;
    }
};
