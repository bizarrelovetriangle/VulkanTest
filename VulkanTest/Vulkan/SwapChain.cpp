#include "SwapChain.h"
#include "..\VulkanContext.h"
#include "DeviceController.h"
#include "Data/ImageData.h"
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#undef LoadImage;

SwapChain::SwapChain(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	CreateSwapChain();
	CreateImageViews();
}

SwapChain::~SwapChain() = default;

void SwapChain::Dispose()
{
	depthBuffer->Dispose();

	for (auto& imageView : swapChainImageViews) {
		vulkanContext.deviceController->device.destroyImageView(imageView);
	}

	for (auto& swapChainFramebuffer : swapChainFramebuffers) {
		vulkanContext.deviceController->device.destroyFramebuffer(swapChainFramebuffer);
	}

	vulkanContext.deviceController->device.destroySwapchainKHR(swapChain);
}

void SwapChain::CreateFramebuffers(vk::RenderPass& renderPass) {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vk::ImageView attachments[] = {
			swapChainImageViews[i],
			depthBuffer->imageView
		};

		vk::FramebufferCreateInfo framebufferInfo(
			{}, renderPass, attachments,
			swapChainExtent.width, swapChainExtent.height, 1);

		swapChainFramebuffers[i] = vulkanContext.deviceController->device.createFramebuffer(framebufferInfo);
	}
}

void SwapChain::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkanContext.deviceController->physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	frameCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && frameCount > swapChainSupport.capabilities.maxImageCount) {
		frameCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo(
		{}, vulkanContext.surface, frameCount, surfaceFormat.format, surfaceFormat.colorSpace,
		extent, 1, vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive, {},
		swapChainSupport.capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode,
		VK_TRUE, VK_NULL_HANDLE);

	swapChain = vulkanContext.deviceController->device.createSwapchainKHR(createInfo);
	swapChainImages = vulkanContext.deviceController->device.getSwapchainImagesKHR(swapChain);

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	{
		Vector2f resolution(extent.width, extent.height);
		depthBuffer = std::make_unique<ImageData>(vulkanContext,
			resolution, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth,
			MemoryType::DeviceLocal);
		depthBuffer->TransitionLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
	}
}

void SwapChain::CreateImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::ComponentMapping components{};
		vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		vk::ImageViewCreateInfo createInfo(
			{}, swapChainImages[i], vk::ImageViewType::e2D, swapChainImageFormat,
			components, subresourceRange);

		swapChainImageViews[i] = vulkanContext.deviceController->device.createImageView(createInfo);
	}
}

vk::Viewport SwapChain::CreateViewport()
{
	vk::Viewport viewport(
		0., (float)swapChainExtent.height,
		(float)swapChainExtent.width, -(float)swapChainExtent.height,
		0., 1.);
	return viewport;
}

vk::Rect2D SwapChain::CreateScissors()
{
	vk::Rect2D scissors({}, swapChainExtent);
	return scissors;
}

SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice)
{
	SwapChainSupportDetails details
	{
		.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(vulkanContext.surface),
		.formats = physicalDevice.getSurfaceFormatsKHR(vulkanContext.surface),
		.presentModes = physicalDevice.getSurfacePresentModesKHR(vulkanContext.surface)
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
	glfwGetFramebufferSize(vulkanContext.window, &width, &height);

	vk::Extent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}
