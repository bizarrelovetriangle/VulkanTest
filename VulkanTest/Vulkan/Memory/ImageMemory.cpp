#include "ImageMemory.h"
#include "..\..\VulkanContext.h"
#include "..\DeviceController.h"
#include "BufferMemory.h"

ImageMemory::ImageMemory(VulkanContext& vulkanContext,
	vk::Extent3D extend, vk::Format format, vk::ImageUsageFlags usage,
	MemoryType memoryType)
	: DeviceMemory(vulkanContext, memoryType)
{
	auto& device = vulkanContext.deviceController->device;

	auto tiling = memoryType == MemoryType::DeviceLocal ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

	vk::ImageCreateInfo createImageInfo(
		{}, vk::ImageType::e2D, format, extend, 1, 1, vk::SampleCountFlagBits::e1,
		tiling, usage, vk::SharingMode::eExclusive);

	image = device.createImage(createImageInfo);

	auto memoryRequirements = device.getImageMemoryRequirements(image);
	AllocateMemory(memoryRequirements);
	device.bindImageMemory(image, memory, 0);
}

void ImageMemory::StagingFlush(std::span<std::byte> data)
{

}

void ImageMemory::Dispose()
{
	vulkanContext.deviceController->device.destroyImage(image);
	DeviceMemory::Dispose();
}


void ImageMemory::LoadImage()
{
	//vulkanContext.deviceController->device.destroyImage(image);
	//DeviceMemory::Dispose();
}
