#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "SwapChain.h"
#include "DeviceController.h"
#include <algorithm>

SwapChain::SwapChain(std::shared_ptr<DeviceController> deviceController, vk::SurfaceKHR& surface, GLFWwindow* window)
    : deviceController(deviceController), surface(surface), window(window)
{
    createSwapChain();
    createImageViews();
}

void SwapChain::Dispose()
{
    for (auto& imageView : swapChainImageViews) {
        deviceController->device.destroyImageView(imageView);
    }

    for (auto& swapChainFramebuffer : swapChainFramebuffers) {
        deviceController->device.destroyFramebuffer(swapChainFramebuffer);
    }

    deviceController->device.destroySwapchainKHR(swapChain);
}

void SwapChain::CreateFramebuffers(vk::RenderPass& renderPass) {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vk::ImageView attachments[] = {
            swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo
        {
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1
        };

        swapChainFramebuffers[i] = deviceController->device.createFramebuffer(framebufferInfo);
    }
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(deviceController->physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo
    {
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,

        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0, // Optional
        .pQueueFamilyIndices = nullptr, // Optional
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    swapChain = deviceController->device.createSwapchainKHR(createInfo);
    swapChainImages = deviceController->device.getSwapchainImagesKHR(swapChain);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vk::ComponentMapping components
        {
            .r = vk::ComponentSwizzle::eIdentity,
            .g = vk::ComponentSwizzle::eIdentity,
            .b = vk::ComponentSwizzle::eIdentity,
            .a = vk::ComponentSwizzle::eIdentity,
        };

        vk::ImageSubresourceRange subresourceRange
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        vk::ImageViewCreateInfo createInfo
        {
            .image = swapChainImages[i],
            .viewType = vk::ImageViewType::e2D,
            .format = swapChainImageFormat,
            .components = components,
            .subresourceRange = subresourceRange
        };

        swapChainImageViews[i] = deviceController->device.createImageView(createInfo);
    }
}

SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice)
{
    SwapChainSupportDetails details
    {
        .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface),
        .formats = physicalDevice.getSurfaceFormatsKHR(surface),
        .presentModes = physicalDevice.getSurfacePresentModesKHR(surface)
    };
    return details;
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    return vk::PresentModeKHR::eImmediate;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    vk::Extent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}
