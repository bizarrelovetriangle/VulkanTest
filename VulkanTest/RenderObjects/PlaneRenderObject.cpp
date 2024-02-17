#include "PlaneRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"

PlaneRenderObject::PlaneRenderObject(VulkanContext& vulkanContext)
	: RenderObject(vulkanContext)
{
	std::span<PlaneObjectUniform> uniformSpan(&evenPlaneObjectUniform, &evenPlaneObjectUniform + 1);
	evenPlaneObjectUniformBuffer = BufferData::Create<PlaneObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	shared = Shared<PlaneRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*evenPlaneObjectUniformBuffer, 2);
}

PlaneRenderObject::~PlaneRenderObject() = default;

void PlaneRenderObject::UpdatePlaneUniformBuffer()
{
	std::span<PlaneObjectUniform> uniformSpan(&evenPlaneObjectUniform, &evenPlaneObjectUniform + 1);
	evenPlaneObjectUniformBuffer->FlushData(uniformSpan);
}

void PlaneRenderObject::Dispose()
{
	evenPlaneObjectUniformBuffer->Dispose();
	RenderObject::Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> PlaneRenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void PlaneRenderObject::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}
