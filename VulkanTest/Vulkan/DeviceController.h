#pragma once
#include <vulkan/vulkan.hpp>
#include "ValidationLayersInfo.h"
#include <vector>

class QueueFamilies;

class DeviceController
{
public:
    DeviceController(vk::Instance& instance, ValidationLayersInfo& validationLayersInfo);
    void createDevice(QueueFamilies& queueFamilies, std::vector<uint32_t> queueFamilyIndexes);
    void Dispose();

private:
    void pickPhysicalDevice();

public:
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

private:
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::Instance& instance;
    ValidationLayersInfo validationLayersInfo;
};