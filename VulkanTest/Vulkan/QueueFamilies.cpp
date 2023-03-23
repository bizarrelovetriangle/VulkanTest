#pragma once

#include "QueueFamilies.h"
#include "DeviceController.h"
#include <set>

QueueFamilies::QueueFamilies(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface)
	: physicalDevice(physicalDevice), surface(surface)
{
    findQueueFamilies();
}

void QueueFamilies::findQueueFamilies() {
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
