#include "DeviceController.h"
#include "ValidationLayersInfo.h"
#include <vector>
#include <stdexcept>
#include "QueueFamilies.h"
#include <vulkan/vulkan.hpp>

DeviceController::DeviceController(vk::Instance& instance, ValidationLayersInfo& validationLayersInfo)
    : instance(instance), validationLayersInfo(validationLayersInfo)
{
    pickPhysicalDevice();
}

void DeviceController::createDevice(QueueFamilies& queueFamilies, std::vector<uint32_t> queueFamilyIndexes)
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueFamilyIndexes.erase(std::unique(std::begin(queueFamilyIndexes), std::end(queueFamilyIndexes)), std::end(queueFamilyIndexes));

    for (auto index : queueFamilyIndexes) {
        float queuePriority[] = { 1.0f };
        vk::DeviceQueueCreateInfo queueCreateInfo({}, index, queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.fillModeNonSolid = true;

    vk::DeviceCreateInfo createInfo({}, queueCreateInfos, {}, {}, &deviceFeatures);

    if (validationLayersInfo.enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayersInfo.validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayersInfo.validationLayers.data();
    }

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    device = physicalDevice.createDevice(createInfo);
}

void DeviceController::Dispose() {
    device.destroy();
}

void DeviceController::pickPhysicalDevice() {
    auto physicalDevices = instance.enumeratePhysicalDevices();

    for (const auto& device : physicalDevices) {
        auto deviceProperties = device.getProperties();

        auto deviceType = vk::PhysicalDeviceType::eIntegratedGpu;
        //auto deviceType = vk::PhysicalDeviceType::eDiscreteGpu;

        if (deviceProperties.deviceType == deviceType) {
            physicalDevice = device;
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}
