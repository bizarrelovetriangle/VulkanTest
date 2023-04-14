#include "QueueFamilies.h"
#include "DeviceController.h"
#include <set>

QueueFamilies::QueueFamilies(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface)
	: physicalDevice(physicalDevice), surface(surface)
{
    findQueueFamilies();

    auto graphicQueueFamily_ = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [](auto& family) { return family.flags.contains(vk::QueueFlagBits::eGraphics); });

    auto presentQueueFamily_ = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [](auto& family) { return family.presentSupport; });

    auto transferQueueFamily_ = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
        [graphicQueueFamily_, presentQueueFamily_](auto& family)
        {
            auto pipelineQueue = family.index == graphicQueueFamily_->index || family.index == presentQueueFamily_->index;
            return !pipelineQueue && family.flags.contains(vk::QueueFlagBits::eTransfer);
        });

    if (transferQueueFamily_ == std::end(queueFamilies))
    {
        transferQueueFamily_ = std::find_if(std::begin(queueFamilies), std::end(queueFamilies),
            [](auto& family) { return family.flags.contains(vk::QueueFlagBits::eTransfer); });
    }

    graphicQueueFamily = graphicQueueFamily_->index;
    presentQueueFamily = presentQueueFamily_->index;
    transferQueueFamily = transferQueueFamily_->index;
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
