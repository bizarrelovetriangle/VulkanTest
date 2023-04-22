#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "SwapChain.h"
#include "Memory/BufferMemory.h"
#include "Memory/ImageMemory.h"
#include "../Primitives/RenderObject.h"

DescriptorSets::DescriptorSets(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	size_t count = vulkanContext.swapChain->frameCount;
	auto& device = vulkanContext.deviceController->device;

	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
	{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, count),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, count)
	};

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, count, descriptorPoolSizes);
	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::DescriptorSetLayoutBinding> bindings
	{
		vk::DescriptorSetLayoutBinding(
			0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(
			1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, layouts);
	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void DescriptorSets::UpdateDescriptor(BufferMemory<RenderObjectUniform>& uniform, ImageMemory& imageMemory)
{
	for (size_t i = 0; i < descriptorSets.size(); ++i)
	{
		vk::DescriptorBufferInfo descriptorBufferInfo(
			uniform.buffer, 0, sizeof(RenderObjectUniform));
		vk::DescriptorImageInfo descriptorImageInfo(
			imageMemory.sampler, imageMemory.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);

		std::vector<vk::WriteDescriptorSet> writesDescriptorSet
		{
			vk::WriteDescriptorSet(
				descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo, {}),
			vk::WriteDescriptorSet(
				descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, {}, {})
		};

		vulkanContext.deviceController->device.updateDescriptorSets(writesDescriptorSet, {});
	}
}

void DescriptorSets::Dispose()
{
	vulkanContext.deviceController->device.destroyDescriptorSetLayout(descriptorSetLayout);
	vulkanContext.deviceController->device.destroyDescriptorPool(descriptorPool);
}