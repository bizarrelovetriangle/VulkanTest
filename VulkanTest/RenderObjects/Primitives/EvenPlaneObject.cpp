#include "EvenPlaneObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Vulkan/Data/BufferData.h"

EvenPlaneObject::EvenPlaneObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& normal)
	: RenderObject(vulkanContext), plane(position, normal)
{
	uniform.color = Vector4f(1.,1.,1.,0.1);

	model = plane.getMatrix();

	std::span<EvenPlaneObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferData>(BufferData::Create<EvenPlaneObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));

	shared = Shared<EvenPlaneObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}

void EvenPlaneObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}
