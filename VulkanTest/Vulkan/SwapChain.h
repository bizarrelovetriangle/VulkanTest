#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class VulkanContext;
class ImageData;

class SwapChain {
public:
	SwapChain(VulkanContext& vulkanContext);
	~SwapChain();
	void Dispose();
	void CreateFramebuffers(vk::RenderPass& renderPass);

	vk::Viewport CreateViewport();
	vk::Rect2D CreateScissors();

private:
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	void CreateSwapChain();
	void CreateImageViews();

	SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicalDevice);
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

public:
	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::ImageView> swapChainImageViews;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	std::unique_ptr<ImageData> depthBuffer;

	size_t frameCount = 0;
private:
	VulkanContext& vulkanContext;
};