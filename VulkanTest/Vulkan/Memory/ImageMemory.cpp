#include "ImageMemory.h"
#include "..\..\VulkanContext.h"
#include "..\DeviceController.h"
#include "BufferMemory.h"

ImageMemory::ImageMemory(VulkanContext& vulkanContext,
	vk::Extent3D extend, vk::Format format, vk::ImageUsageFlags usage)
	: DeviceMemory(vulkanContext, MemoryType::DeviceLocal)
{
	auto& device = vulkanContext.deviceController->device;

	vk::ImageCreateInfo createImageInfo(
		{}, vk::ImageType::e2D, format, extend, 1, 1, vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal, usage, vk::SharingMode::eExclusive);

	image = device.createImage(createImageInfo);

	auto memoryRequirements = device.getImageMemoryRequirements(image);
	AllocateMemory(memoryRequirements);
	device.bindImageMemory(image, memory, 0);
}

void ImageMemory::StagingFlush(std::span<std::byte>)
{
}

void ImageMemory::Dispose()
{
	vulkanContext.deviceController->device.destroyImage(image);
	DeviceMemory::Dispose();
}
