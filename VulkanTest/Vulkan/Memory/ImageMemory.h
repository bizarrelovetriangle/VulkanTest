#pragma once
#include <vulkan/vulkan.hpp>
#include "DeviceMemory.h"
#include "../../Math/Vector2.hpp"

class VulkanContext;

class ImageMemory : public DeviceMemory
{
public:
	ImageMemory(VulkanContext& vulkanContext,
		const Vector2u& resolution, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags imageAspect,
		MemoryType memoryType);
	void FlushData(std::span<std::byte> data) override;
	void TransitionLayout(const vk::ImageLayout& newImageLayout);
	void CreateImageViewAndSampler();
	void Dispose();

	static std::pair<Vector2u, std::vector<std::byte>> LoadImage(const std::string& path);

public:
	vk::Image image;
	vk::ImageView imageView;
	vk::Sampler sampler;

private:
	Vector2u resolution;
	vk::Format format;
	vk::ImageUsageFlags usage;
	vk::ImageLayout imageLayout;
	vk::ImageType imageType;
	vk::ImageViewType imageViewType;
	vk::ImageAspectFlags imageAspect;
};
