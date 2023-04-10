#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include "ValidationLayersInfo.h"

class VulkanAuxiliary {
public:
    VulkanAuxiliary(ValidationLayersInfo& validationLayersInfo);

    void Dispose();

private:
    void createInstance();
    std::vector<const char*> getRequiredExtensions();

public:
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::Instance instance;
    vk::SurfaceKHR surface;

private:
    ValidationLayersInfo& validationLayersInfo;

    struct DebugDispatch {
        int getVkHeaderVersion() const { return 239; };
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 0;
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = 0;
    } debugDispatch;
};