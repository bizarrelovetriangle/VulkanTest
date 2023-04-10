#include "ImageHelper.h"
#include "..\VulkanContext.h"

void ImageHelper::CreateImage(
    vk::Extent3D extend, vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::Image& image, vk::DeviceMemory& imageMemory)
{
    vk::ImageCreateInfo createImageInfo(
        {}, vk::ImageType::e2D, format, extend, 1, 1, vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal, usage, vk::SharingMode::eExclusive);

    //vk::CreateIn
};