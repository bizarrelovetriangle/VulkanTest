#pragma once
#include <vulkan/vulkan.hpp>
#include "DeviceMemory.h"

class VulkanContext;

class ImageMemory : public DeviceMemory
{
public:
	ImageMemory(VulkanContext& vulkanContext,
		vk::Extent3D extend, vk::Format format, vk::ImageUsageFlags usage);
	void StagingFlush(std::span<std::byte> data) override;
	void Dispose();
public:
	vk::Image image;
};

