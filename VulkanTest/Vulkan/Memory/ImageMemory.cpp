#include "ImageMemory.h"
#include "..\..\VulkanContext.h"
#include "..\DeviceController.h"
#include "BufferMemory.h"
#include <string>
#include "../../Dependencies/stb_image.h"
#include "../../Math/Vector2.hpp"

ImageMemory::ImageMemory(VulkanContext& vulkanContext,
		const Vector2u& resolution, vk::Format format, vk::ImageUsageFlags usage,
		MemoryType memoryType)
	: DeviceMemory(vulkanContext, memoryType), resolution(resolution), format(format)
{
	auto& device = vulkanContext.deviceController->device;

	auto tiling = memoryType == MemoryType::DeviceLocal ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;
	imageType = vk::ImageType::e2D;
	imageViewType = vk::ImageViewType::e2D;

	vk::Extent3D extend(resolution.x, resolution.x, 1);
	vk::ImageCreateInfo createImageInfo(
		{}, vk::ImageType::e2D, format, extend, 1, 1, vk::SampleCountFlagBits::e1,
		tiling, usage, vk::SharingMode::eExclusive);

	image = device.createImage(createImageInfo);

	auto memoryRequirements = device.getImageMemoryRequirements(image);
	AllocateMemory(memoryRequirements);
	device.bindImageMemory(image, memory, 0);

	CreateImageViewAndSampler();
}

void ImageMemory::StagingFlush(std::span<std::byte> data)
{

}

void ImageMemory::TransitionLayout()
{

}

void ImageMemory::CreateImageViewAndSampler()
{
	vk::ComponentMapping components{};
	vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	vk::ImageViewCreateInfo imageViewInfo({}, image, imageViewType, format, components, subresourceRange );
	imageView = vulkanContext.deviceController->device.createImageView(imageViewInfo);

	auto properties = vulkanContext.deviceController->physicalDevice.getProperties();
	float anisotropy = properties.limits.maxSamplerAnisotropy;
	vk::SamplerCreateInfo samplerInfo({}, vk::Filter::eLinear, vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		0., true, anisotropy,
		false, vk::CompareOp::eAlways,
		0., 0.,
		vk::BorderColor::eFloatOpaqueBlack,
		false);
	sampler = vulkanContext.deviceController->device.createSampler(samplerInfo);
}

void ImageMemory::Dispose()
{
	vulkanContext.deviceController->device.destroySampler(sampler);
	vulkanContext.deviceController->device.destroyImageView(imageView);
	vulkanContext.deviceController->device.destroyImage(image);
	DeviceMemory::Dispose();
}

std::pair<Vector2u, std::vector<std::byte>> ImageMemory::LoadImage(const std::string& path)
{
	int width, height, channels;
	std::byte* pixels = (std::byte*) stbi_load(
		path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) throw std::runtime_error("failed to load texture image!");
	size_t imageSize = size_t(width) * height * 4;
	return std::make_pair(Vector2u(width, height), std::vector<std::byte>(pixels, pixels + imageSize));
}
