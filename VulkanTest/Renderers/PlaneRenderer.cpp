#include "PlaneRenderer.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"

PlaneRenderer::PlaneRenderer(VulkanContext& vulkanContext)
	: Renderer(vulkanContext)
{
	std::span<PlaneObjectUniform> uniformSpan(&evenPlaneObjectUniform, &evenPlaneObjectUniform + 1);
	evenPlaneObjectUniformBuffer = BufferData::Create<PlaneObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	shared = Shared<PlaneRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*evenPlaneObjectUniformBuffer, 2);
}

PlaneRenderer::~PlaneRenderer() = default;

void PlaneRenderer::UpdatePlaneUniformBuffer()
{
	std::span<PlaneObjectUniform> uniformSpan(&evenPlaneObjectUniform, &evenPlaneObjectUniform + 1);
	evenPlaneObjectUniformBuffer->FlushData(uniformSpan);
}

void PlaneRenderer::Dispose()
{
	evenPlaneObjectUniformBuffer->Dispose();
	Renderer::Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> PlaneRenderer::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void PlaneRenderer::Accept(RenderVisitor& renderVisitor, const Camera& camera)
{
	renderVisitor.Visit(*this, camera);
}
