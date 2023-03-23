#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_HEADER_VERSION 239
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include "ValidationLayersInfo.h"
#include <vector>
#include <stdexcept>

class QueueFamilies;

class DeviceController
{
public:
    DeviceController(vk::Instance& instance, ValidationLayersInfo& validationLayersInfo);
    void createDevice(QueueFamilies& queueFamilies, std::initializer_list<size_t> queueFamilyIndexes);
    void Dispose();

private:
    void pickPhysicalDevice();

public:
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

private:
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::Instance& instance;
    ValidationLayersInfo& validationLayersInfo;
};