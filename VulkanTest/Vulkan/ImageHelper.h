#pragma once
#include <vulkan/vulkan.hpp>

class VulkanContext;

class ImageHelper
{
public:
	ImageHelper(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{
	}

	void CreateImage(
		vk::Extent3D extend, vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::Image& image, vk::DeviceMemory& imageMemory);

private:
	VulkanContext& vulkanContext;
};

