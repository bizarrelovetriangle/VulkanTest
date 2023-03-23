#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "VulkanAuxiliary.h"
#include "ValidationLayersInfo.h"
#include <vector>
#include <iostream>

namespace
{
    VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT             messageTypes,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }
}

VulkanAuxiliary::VulkanAuxiliary(ValidationLayersInfo& validationLayersInfo)
    : validationLayersInfo(validationLayersInfo)
{
    createInstance();
}

void VulkanAuxiliary::Dispose() {
    if (validationLayersInfo.enableValidationLayers) {
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, debugDispatch);
    }

    instance.destroySurfaceKHR(surface);
    instance.destroy();
}

void VulkanAuxiliary::createInstance()
{
    vk::DebugUtilsMessengerCreateInfoEXT debugMessagerCreateInfo
    {
        .messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback,
    };

    vk::ApplicationInfo appInfo
    {
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo
    {
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = (uint32_t)extensions.size(),
        .ppEnabledExtensionNames = extensions.data()
    };

    if (validationLayersInfo.enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayersInfo.validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayersInfo.validationLayers.data();
        createInfo.pNext = &debugMessagerCreateInfo;
    }

    instance = vk::createInstance(createInfo);

    if (validationLayersInfo.enableValidationLayers) {
        debugDispatch.vkCreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
        debugDispatch.vkDestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");

        debugMessenger = instance.createDebugUtilsMessengerEXT(debugMessagerCreateInfo, nullptr, debugDispatch);
    }
}

std::vector<const char*> VulkanAuxiliary::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}
