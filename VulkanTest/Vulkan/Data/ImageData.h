#pragma once
#include <vulkan/vulkan.hpp>
#include "DeviceMemory/DeviceMemory.h"
#include "../../Math/Vector2.h"
#include "../../Utils/Disposable.h"

class VulkanContext;

class ImageData : public Disposable<ImageData>
{
public:
	ImageData(VulkanContext& vulkanContext,
		const Vector2u& resolution, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags imageAspect,
		MemoryType memoryType);
	void FlushData(std::span<std::byte> data);
	void TransitionLayout(const vk::ImageLayout& newImageLayout);
	void CreateImageViewAndSampler();
	void DisposeAction();

	static std::pair<Vector2u, std::vector<std::byte>> LoadImage(const std::string& path);

public:
	vk::Image image;
	vk::ImageView imageView;
	vk::Sampler sampler;

private:
	VulkanContext& vulkanContext;
	DeviceMemory deviceMemory;

	Vector2u resolution;
	vk::Format format;
	vk::ImageUsageFlags usage;
	vk::ImageLayout imageLayout;
	vk::ImageType imageType;
	vk::ImageViewType imageViewType;
	vk::ImageAspectFlags imageAspect;
};
