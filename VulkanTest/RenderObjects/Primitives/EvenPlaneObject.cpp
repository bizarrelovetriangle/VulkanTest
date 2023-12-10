#include "EvenPlaneObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"

EvenPlaneObject::EvenPlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal)
	: RenderObject(vulkanContext), plane(position, normal)
{
	transformUniform.model = plane.getMatrix();
	UpdateTransformUniformBuffer();

	evenPlaneObjectUniform.color = Vector4f(0.5, 0., 0.5, 1.);
	std::span<EvenPlaneObjectUniform> uniformSpan(&evenPlaneObjectUniform, &evenPlaneObjectUniform + 1);
	evenPlaneObjectUniformBuffer = BufferData::Create<EvenPlaneObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	shared = Shared<EvenPlaneObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*evenPlaneObjectUniformBuffer, 2);
}

EvenPlaneObject::~EvenPlaneObject() = default;

void EvenPlaneObject::Dispose()
{
	RenderObject::Dispose();
	evenPlaneObjectUniformBuffer->Dispose();
}

std::vector<vk::DescriptorSetLayoutBinding> EvenPlaneObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void EvenPlaneObject::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}
