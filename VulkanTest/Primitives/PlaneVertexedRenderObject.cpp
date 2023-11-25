#include "PlaneVertexedRenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../Utils/SingletonManager.h"

PlaneVertexedRenderObject::PlaneVertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
	: VertexedRenderObject(vulkanContext, deserializedObject)
{
	vertexData = std::vector<VertexData>(
		std::begin(deserializedObject.vertexData), std::end(deserializedObject.vertexData));
	vertexBuffer = std::make_unique<BufferData>(BufferData::Create<VertexData>(
		vulkanContext, vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer));

	auto& shared = vulkanContext.singletonManager->Get<Shared<PlaneVertexedRenderObject>>();
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared.descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*uniformBuffer, 0);
}

PlaneVertexedRenderObject::~PlaneVertexedRenderObject() = default;

void PlaneVertexedRenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}
