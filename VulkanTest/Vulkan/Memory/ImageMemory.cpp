#include "ImageMemory.h"
#include "..\..\VulkanContext.h"
#include "..\DeviceController.h"
#include "BufferMemory.h"
#include <string>
#include "../../Dependencies/stb_image.h"
#include "../../Math/Vector2.hpp"
#include "../CommandBufferDispatcher.h"
#include "../QueueFamilies.h"

ImageMemory::ImageMemory(VulkanContext& vulkanContext,
		const Vector2u& resolution, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags imageAspect,
		MemoryType memoryType)
	: DeviceMemory(vulkanContext, memoryType), resolution(resolution), format(format), usage(usage), imageAspect(imageAspect)
{
	auto& device = vulkanContext.deviceController->device;
	vk::ImageTiling tiling{};

	if (memoryType == MemoryType::DeviceLocal) {
		imageLayout = vk::ImageLayout::eUndefined;
		tiling = vk::ImageTiling::eOptimal;
		usage |= vk::ImageUsageFlagBits::eTransferDst;
	}
	else {
		imageLayout = vk::ImageLayout::ePreinitialized;
		tiling = vk::ImageTiling::eLinear;
	}

	imageType = vk::ImageType::e2D;
	imageViewType = vk::ImageViewType::e2D;

	vk::Extent3D extent(resolution.x, resolution.x, 1);
	vk::ImageCreateInfo createImageInfo(
		{}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1,
		tiling, usage, vk::SharingMode::eExclusive, {}, imageLayout);

	image = device.createImage(createImageInfo);

	auto memoryRequirements = device.getImageMemoryRequirements(image);
	AllocateMemory(memoryRequirements);
	device.bindImageMemory(image, memory, 0);

	CreateImageViewAndSampler();
}

void ImageMemory::FlushData(std::span<std::byte> data)
{
	if (memoryType == MemoryType::Universal || memoryType == MemoryType::HostLocal)
	{
		DeviceMemory::FlushData(data);
		return;
	}

	ImageMemory stagingImage(vulkanContext, resolution, format, usage | vk::ImageUsageFlagBits::eTransferSrc, imageAspect,
		MemoryType::HostLocal);
	stagingImage.FlushData(data);
	stagingImage.TransitionLayout(vk::ImageLayout::eTransferSrcOptimal);
	TransitionLayout(vk::ImageLayout::eTransferDstOptimal);

	uint32_t queueFamily = vulkanContext.queueFamilies->graphicQueueFamily;
	vulkanContext.commandBufferDispatcher->Invoke(queueFamily, [this, &stagingImage](vk::CommandBuffer& cb)
		{
			vk::ImageSubresourceLayers subresourceLayers(imageAspect, 0, 0, 1);
			vk::Extent3D extent(resolution.x, resolution.x, 1);
			vk::ImageCopy region(subresourceLayers, {}, subresourceLayers, {}, extent);
			cb.copyImage(stagingImage.image, stagingImage.imageLayout, this->image, this->imageLayout, region);
		});

	stagingImage.Dispose();
}

void ImageMemory::TransitionLayout(const vk::ImageLayout& newImageLayout)
{
	uint32_t queueFamily = vulkanContext.queueFamilies->graphicQueueFamily;
	vulkanContext.commandBufferDispatcher->Invoke(queueFamily, [this, &newImageLayout](auto& cb)
		{
			vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			vk::AccessFlags srcAccess{};
			vk::PipelineStageFlags dstStage{};
			vk::AccessFlags dstAccess{};

			if (imageLayout == vk::ImageLayout::eUndefined) {}
			if (imageLayout == vk::ImageLayout::ePreinitialized) {}

			if (imageLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
				srcStage = vk::PipelineStageFlagBits::eFragmentShader;
				srcAccess = vk::AccessFlagBits::eShaderRead;
			}

			if (imageLayout == vk::ImageLayout::eTransferDstOptimal) {
				srcStage = vk::PipelineStageFlagBits::eTransfer;
				srcAccess = vk::AccessFlagBits::eTransferWrite;
			}

			if (newImageLayout == vk::ImageLayout::eTransferSrcOptimal) {
				dstStage = vk::PipelineStageFlagBits::eTransfer;
				dstAccess = vk::AccessFlagBits::eTransferRead;
			}

			if (newImageLayout == vk::ImageLayout::eTransferDstOptimal) {
				dstStage = vk::PipelineStageFlagBits::eTransfer;
				dstAccess = vk::AccessFlagBits::eTransferWrite;
			}

			if (newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
				dstStage = vk::PipelineStageFlagBits::eFragmentShader;
				dstAccess = vk::AccessFlagBits::eShaderRead;
			}

			if (newImageLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal) {
				dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
				dstAccess = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			}

			vk::ImageSubresourceRange subresourceRange(imageAspect, 0, 1, 0, 1);
			vk::ImageMemoryBarrier imageBarrier(
				srcAccess, dstAccess,
				imageLayout, newImageLayout,
				{}, {},
				image, subresourceRange);

			cb.pipelineBarrier(srcStage, dstStage, {}, {}, {}, imageBarrier);
		});

	imageLayout = newImageLayout;
}

void ImageMemory::CreateImageViewAndSampler()
{
	vk::ComponentMapping components{};
	vk::ImageSubresourceRange subresourceRange(imageAspect, 0, 1, 0, 1);
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
