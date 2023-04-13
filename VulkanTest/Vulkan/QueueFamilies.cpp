#include "QueueFamilies.h"
#include "DeviceController.h"
#include <set>

QueueFamilies::QueueFamilies(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface)
	: physicalDevice(physicalDevice), surface(surface)
{
    findQueueFamilies();

    auto graphicQueueFamily = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [](auto& family) { return family.flags.contains(vk::QueueFlagBits::eGraphics); });

    auto presentQueueFamily = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [](auto& family) { return family.presentSupport; });

    auto transferFamilyQueue = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [graphicQueueFamily, presentQueueFamily](auto& family)
        {
            auto pipelineQueue = family.index == graphicQueueFamily->index || family.index == presentQueueFamily->index;
            return !pipelineQueue && family.flags.contains(vk::QueueFlagBits::eTransfer);
        });

    if (transferFamilyQueue == std::end(queueFamilies))
    {
        transferFamilyQueue = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
            [](auto& family) { return family.flags.contains(vk::QueueFlagBits::eTransfer); });
    }

    graphicQueueFamilyIndex = graphicQueueFamily->index;
    presentQueueFamilyIndex = presentQueueFamily->index;
    transferQueueFamilyIndex = transferFamilyQueue->index;
}

void QueueFamilies::findQueueFamilies()
{
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
        const auto& queueFamilyProperty = queueFamilyProperties[i];
        vk::Bool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

        QueueFamily queueFamily
        {
            .count = queueFamilyProperty.queueCount,
            .index = i,
            .presentSupport = presentSupport > 0
        };

        for (auto& flag : queueFlags) {
            if (queueFamilyProperty.queueFlags & flag) {
                queueFamily.flags.insert(flag);
            }
        }

        queueFamilies.push_back(queueFamily);
    }
}
